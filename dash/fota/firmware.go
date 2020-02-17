package fota

import (
	"encoding/json"
	"fmt"
	"os"
	"path"
)

// Firmware represents a firmware
type Firmware struct {
	// Type represents the type of firmware
	Type string `json:"type"` // "type": "espBME280"

	// Version the current auto increseing version number
	// When devices boot they will be checking their own firmware number
	// against this one
	Version int `json:"version"` // "version": 2,

	// Where should the device fetch a current firmware from...
	Host string `json:"host"` // "host": "192.168.0.100",
	Port int    `json:"port"` // "port": 80,
	Bin  string `json:"bin"`  // "bin": "/files/esp32-fota-http-2.bin"
}

// Database represents database of type Firmware - it just uses the filesystem
// and will prove prone to racing errors
type Database string

// Update updates firmware metadatas
func (d Database) Update(name string, f *Firmware) error {
	// notice that os.Create truncates existing files
	fd, err := os.Create(path.Join(string(d), name))
	if err != nil {
		return fmt.Errorf("could not create file: %w", err)
	}

	encoder := json.NewEncoder(fd)
	err = encoder.Encode(f)
	if err != nil {
		return fmt.Errorf("could not save json: %w", err)
	}

	return nil
}

// Find fetches JSON from the filesystem and decodes it into Firmware structs
// users should be aware when looking up a firmware which does not exist it will be created
func (d Database) Find(name string) (*Firmware, error) {
	f := path.Join(string(d), name)
	fd, err := os.Open(f)

	// we treat Not Exists as an opportunity to create a new one
	if os.IsNotExist(err) {
		return &Firmware{Type: name}, nil
	}

	// otherwise something is really wrong
	if err != nil {
		return nil, fmt.Errorf("unable to open %s: %w", f, err)
	}

	decoder := json.NewDecoder(fd)
	firmware := &Firmware{}

	err = decoder.Decode(firmware)
	if err != nil {
		return nil, fmt.Errorf("unable to unmarshal database %s: %w", f, err)
	}

	return firmware, nil
}
