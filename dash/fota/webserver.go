package fota

import (
	"crypto/rand"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"path"
)

// Webserver provides firmware update services
type Webserver struct {
	http.Server

	// StorePath is where binary firemware files will be located
	// and served from
	StorePath string

	// DatabasePath is a directory containing json files describing
	// what firmware a device type should use
	DatabasePath string

	// PublishFirmware allows "announce" endpoint to publish firmwares
	PublishFirmware func(string, string) error
}

// Setup creates needed directories, applies http handlers and and sane defaults
func (w *Webserver) Setup() error {

	if w.StorePath == "" {
		w.StorePath = "/firmware"
	}
	if w.DatabasePath == "" {
		w.DatabasePath = "/database"
	}

	if w.PublishFirmware == nil {
		w.PublishFirmware = func(_, _ string) error {
			log.Printf("programmer did not supply a PublishFirmware method...")
			return nil
		}
	}

	err := ensureDirectory(w.StorePath)
	if err != nil {
		return fmt.Errorf("unable to use %s: %w", w.StorePath, err)
	}

	err = ensureDirectory(w.DatabasePath)
	if err != nil {
		return fmt.Errorf("unable to use %s: %w", w.DatabasePath, err)
	}

	mux := http.NewServeMux()

	// serve files directly from store and database
	mux.Handle("/files/", http.StripPrefix("/files/", http.FileServer(http.Dir(w.StorePath))))
	mux.Handle("/db/", http.StripPrefix("/db/", http.FileServer(http.Dir(w.DatabasePath))))

	mux.HandleFunc("/firmware", w.acceptFirmware)
	mux.HandleFunc("/announce", w.announceFirmware)

	w.Server.Handler = mux

	return nil
}

// publishFirmware endpoint is called like this:
// PUT /publish/<espType>/<firmware-name>
// E.g. /publish/espBME280/9F6D45CF23BCA7EEA6D962F9AA7BF4DB
// where espBME280 is something platformio and the devices them selves should know about
// and the hash is returned from "POST /firmware" call's
func (s *Webserver) announceFirmware(w http.ResponseWriter, r *http.Request) {
	// All information needed is found in the url
	r.Body.Close()

	// we need to know the current firmware version

}

func (s *Webserver) acceptFirmware(w http.ResponseWriter, r *http.Request) {
	defer r.Body.Close()

	if r.Method != http.MethodPost {
		http.Error(w, "not a post request", http.StatusBadRequest)
		return
	}

	// lets have a unique name
	name, err := randomFilename()
	if err != nil {
		http.Error(w, "i am severely broken, check logs", http.StatusInternalServerError)
		log.Printf("could not generate random filename for firmware: %s", err)
		return
	}

	// lets create the file
	fd, err := os.Create(path.Join(s.StorePath, name))
	if err != nil {
		http.Error(w, "i am severely broken, check logs", http.StatusInternalServerError)
		log.Printf("could not create file for writing: %s", err)
		return
	}

	// lets copy the http body to the file
	n, err := io.Copy(fd, r.Body)
	if err != nil {
		http.Error(w, "i am severely broken, check logs", http.StatusInternalServerError)
		log.Printf("could not copy: %s", err)
		return
	}

	log.Printf("Accepted firmware %s with size %d", name, n)

	// last but not least, tell client what filename this firmware have been given
	fmt.Fprintf(w, "%s", name)
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
