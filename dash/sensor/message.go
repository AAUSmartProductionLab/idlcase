package sensor

import (
	"fmt"
	"time"
)

// Message represents a sensor message
type Message struct {
	DeviceID string
	Type     string
	At       time.Time

	Measurement struct {
		Precision *int
		Value     float64
		Unit      string
	}
}

func (m *Message) Metric() string {
	return fmt.Sprintf("%s/%s", m.DeviceID, m.Type)
}

func (m *Message) Precision() int {
	if m.Measurement.Precision == nil {
		return 2
	}
	return *m.Measurement.Precision
}
