package sensor

import "time"

// Message represents a sensor message
type Message struct {
	DeviceID    string
	Type        string
	At          time.Time
	Measurement struct {
		Value float64
		Unit  string
	}
}
