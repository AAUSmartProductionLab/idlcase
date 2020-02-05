package fota

import (
	"fmt"
	"net/http"
	"os"
)

// Webserver provides firmware update services
type Webserver struct {
	http.Server

	storePath string
}

// NewWebserver returns a webserver which will accept firmware upgrades
// and use `path` as storage
func NewWebserver(path string) (*Webserver, error) {
	err := ensureDirectory(path)
	if err != nil {
		return nil, fmt.Errorf("unable to use %s: %w", path, err)
	}

	mux := http.NewServeMux()
	server := &Webserver{
		Server:    http.Server{Handler: mux},
		storePath: path,
	}

	mux.Handle("/files/", http.StripPrefix("/files/", http.FileServer(http.Dir(path))))
	mux.HandleFunc("/firmware", server.acceptFirmware)

	return server, nil
}

func (s *Webserver) acceptFirmware(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "not a post request", http.StatusBadRequest)
		return
	}

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
