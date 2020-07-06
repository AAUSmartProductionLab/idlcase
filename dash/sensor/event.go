package sensor

import "fmt"

// Event represents an event happening
type Event struct {
	Message
	// Payload is used for machine readable content
	Payload string

	// Msg is a more human readable content
	Msg string
}

func (m *Event) Metric() string {
	return fmt.Sprintf("%s/%s", m.deviceID, "event")
}
