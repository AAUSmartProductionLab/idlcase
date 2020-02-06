package fota

import (
	"encoding/json"
	"fmt"
	"os"
	"path"
)

type Firmware struct {
	Type    string `json:"type"`    // "type": "esp32-fota-http",
	Version int    `json:"version"` // "version": 2,
	Host    string `json:"host"`    // "host": "192.168.0.100",
	Port    int    `json:"port"`    // "port": 80,
	Bin     string `json:"bin"`     // "bin": "/files/esp32-fota-http-2.bin"
}

type Database string

func (d Database) Find(name string) (*Firmware, error) {
	f := path.Join(d, name)
	fd, err := os.Open(f)
	if err != nil {
		return nil, fmt.Errorf("unable to open %s: %w", f, err)
	}

	decoder := json.NewDecoder(fd)

}
