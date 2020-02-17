package fota

import (
	"crypto/rand"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"path"
	"strings"
)

// FIXME: we should be able to figure these out somehow
const fixedHostname = "10.13.37.1"
const fixedPort = 80

// Webserver provides firmware update services
type Webserver struct {
	http.Server

	// StorePath is where binary firemware files will be located
	// and served from
	StorePath string

	// Database is a directory containing json files describing
	// what firmware a device type should use
	Database Database

	// PublishFirmware allows "announce" endpoint to publish firmwares
	// it only accepts a string which should be the firmware Type.
	PublishFirmware func(string) error
}

// Setup creates needed directories, applies http handlers and and sane defaults
func (w *Webserver) Setup() error {

	if w.StorePath == "" {
		w.StorePath = "/firmware"
	}
	if string(w.Database) == "" {
		w.Database = "/database"
	}

	if w.PublishFirmware == nil {
		w.PublishFirmware = func(_ string) error {
			log.Printf("You, the programmer, did not supply a PublishFirmware method...")
			return nil
		}
	}

	err := ensureDirectory(w.StorePath)
	if err != nil {
		return fmt.Errorf("unable to use %s: %w", w.StorePath, err)
	}

	err = ensureDirectory(string(w.Database))
	if err != nil {
		return fmt.Errorf("unable to use %s: %w", string(w.Database), err)
	}

	mux := http.NewServeMux()

	// serve files directly from store and database
	mux.Handle("/files/", http.StripPrefix("/files/", http.FileServer(http.Dir(w.StorePath))))
	mux.Handle("/db/", http.StripPrefix("/db/", http.FileServer(http.Dir(string(w.Database)))))

	mux.HandleFunc("/firmware", w.acceptFirmware)
	mux.HandleFunc("/announce/", w.announceFirmware)
	mux.HandleFunc("/hello", w.hello)

	w.Server.Handler = mux

	return nil
}

func (w *Webserver) hello(rw http.ResponseWriter, r *http.Request) {
	fmt.Fprint(rw, "Hello")
}

// announceFirmware endpoint is called like this:
// PUT /announce/<espType>/<firmwareName>
// announce does the following:
//
// * Checks if this announce request is sane
// * Finds the firmware.json file from fs
// * Bumps this firmware.json file's version number
// * Puts the firmware.json file disk for future devices
// * Announches though PublishFirmware that a new firmware is available
func (w *Webserver) announceFirmware(rw http.ResponseWriter, r *http.Request) {
	// All information needed is found in the url
	r.Body.Close()

	// lets figure out stuff based on the url
	parts := strings.Split(r.URL.Path, "/")

	if len(parts) != 4 {
		http.Error(rw, "url should contain /announce/type/firmwarename", http.StatusBadRequest)
		log.Printf("bogus announce request: %+v", parts)
		return
	}

	fType := parts[2]
	fName := parts[3]

	// more sanity: lets see if this firmware name acturally exists on disk
	_, err := os.Stat(path.Join(w.StorePath, fName))
	if err != nil {
		http.Error(rw, "error looking for the firmware, check logs - did you spell the firmware name correctly?", http.StatusInternalServerError)
		log.Printf("could not find firmware which a client tried to announce %s: %s", fName, err)
	}

	firmware, err := w.Database.Find(fType)
	if err != nil {
		http.Error(rw, "something went wrong while trying to fetch firmware from the database, check logs", http.StatusInternalServerError)
		log.Printf("could not find firmware meta data: %s", err)
		return
	}

	firmware.Version++
	firmware.Host = fixedHostname
	firmware.Port = fixedPort
	firmware.Bin = fmt.Sprintf("/files/%s", fName)

	err = w.Database.Update(fType, firmware)
	if err != nil {
		http.Error(rw, "could not update firmware metadata, check logs", http.StatusInternalServerError)
		log.Printf("could not update firmware meta data %s: %s", fType, err)
		return
	}

	// now it should just be a matter of publishing the new firmware
	err = w.PublishFirmware(fmt.Sprintf("idlota/%s", fType))
	if err != nil {
		// we are going with a status OK - the firmware will be available for clients - it was just not published
		http.Error(rw, "the firmware was updated, but i was unable to Publish this to clients", http.StatusOK)
		log.Printf("could not publish new firmware: %s", err)
		return
	}

	fmt.Fprint(rw, "OK")

}

// acceptFirmware creates a random filename and simply copies the http body into the new file
func (w *Webserver) acceptFirmware(rw http.ResponseWriter, r *http.Request) {
	defer r.Body.Close()

	if r.Method != http.MethodPost {
		http.Error(rw, "not a post request", http.StatusBadRequest)
		return
	}

	// lets have a unique name
	name, err := randomFilename()
	if err != nil {
		http.Error(rw, "i am severely broken, check logs", http.StatusInternalServerError)
		log.Printf("could not generate random filename for firmware: %s", err)
		return
	}

	// lets create the file
	fd, err := os.Create(path.Join(w.StorePath, name))
	if err != nil {
		http.Error(rw, "i am severely broken, check logs", http.StatusInternalServerError)
		log.Printf("could not create file for writing: %s", err)
		return
	}

	// lets copy the http body to the file
	n, err := io.Copy(fd, r.Body)
	if err != nil {
		http.Error(rw, "i am severely broken, check logs", http.StatusInternalServerError)
		log.Printf("could not copy: %s", err)
		return
	}

	log.Printf("Accepted firmware %s with size %d", name, n)

	// last but not least, tell client what filename this firmware have been given
	fmt.Fprintf(rw, "%s", name)
}

func randomFilename() (string, error) {
	b := make([]byte, 16)
	_, err := rand.Read(b)
	if err != nil {
		return "", fmt.Errorf("unable to read random bytes: %w", err)
	}

	return fmt.Sprintf("%X", b), nil
}

func ensureDirectory(p string) error {
	info, err := os.Stat(p)
	if os.IsNotExist(err) {
		// create it
		return os.MkdirAll(p, 0644)
	}

	if !info.IsDir() {
		return fmt.Errorf("%s is not a directory", p)
	}

	return nil
}
