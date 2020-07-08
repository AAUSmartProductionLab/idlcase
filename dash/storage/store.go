package storage

import (
	"fmt"

	"bitbucket.org/ragroup/idlcase/dash/transport"
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
// FIXME: at some point, batching and store it every second or so
func (s *Store) Add(m transport.Message) error {
	point, err := m.Point()
	if err != nil {
		return fmt.Errorf("unable to create point from Message: %w", err)
	}

	bp, _ := client.NewBatchPoints(client.BatchPointsConfig{})
	bp.SetDatabase("idl")
	bp.AddPoint(point)

	err = s.Write(bp)
	if err != nil {
		return fmt.Errorf("unable to store data: %w", err)
	}

	return nil
}
