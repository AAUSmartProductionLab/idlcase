package storage

import (
	"fmt"

	"bitbucket.org/ragroup/idlcase/dash/sensor"

	client "github.com/influxdata/influxdb1-client/v2"
)

// Store handles communication with our database
type Store struct {
	client.Client
}

// NewStore returns a ready to go store
func NewStore() (*Store, error) {
	c, err := client.NewHTTPClient(client.HTTPConfig{
		Addr: "http://localhost:8086",
	})
	if err != nil {
		return nil, fmt.Errorf("unable to create influxdb client: %w", err)
	}

	s := Store{Client: c}
	return &s, nil
}

// Add adds a received sensor message for storage
func (s *Store) Add(m sensor.Message) error {
	point, err := m.ToPoint()
	if err != nil {
		return fmt.Errorf("unable to create point from Message: %w", err)
	}

	// Create a new point batch
	bp, _ := client.NewBatchPoints(client.BatchPointsConfig{
		Database: "idl",
	})
	bp.AddPoint(point)

	err = s.Write(bp)
	if err != nil {
		return fmt.Errorf("unable to store data: %w", err)
	}

	return nil
}
