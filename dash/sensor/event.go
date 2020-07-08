package sensor

import (
	"fmt"

	client "github.com/influxdata/influxdb1-client/v2"
)

// Event represents an event happening
type Event struct {
	Message
	// Payload is used for machine readable content
	Payload string

	// Msg is a more human readable content
	Msg string
}

func (m *Event) Metric() string {
	return fmt.Sprintf("%s/%s", m.deviceID, m.Table)
}

func (e *Event) UIValue() string {
	return e.Msg
}

func (e *Event) UIUnit() string {
	return ""
}

func (e *Event) Point() (*client.Point, error) {

	point, err := client.NewPoint(e.Table, e.Tags, map[string]interface{}{
		"payload": e.Payload,
		"msg":     e.Msg,
	}, *e.At)
	if err != nil {
		return nil, fmt.Errorf("unable to create influxdb point: %s", err)
	}

	return point, nil
}
