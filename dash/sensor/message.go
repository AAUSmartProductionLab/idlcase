package sensor

import (
	"time"
)

// Message represents the intersection of all sensor types and should be embedded
type Message struct {
	deviceID string // e.g. BEEEEF - extracted from mqtt topic
	Table    string // e.g. temperature - extracted from mqtt message
	At       *time.Time

	Tags map[string]string
}

// Since returns a human readable color formatted duration
func (m *Message) Since() string {
	since := time.Now().Sub(*m.At).Round(time.Second)

	return since.String()
}

func (m *Message) DeviceID() string {
	return m.deviceID
}

func (m *Message) SetDeviceID(d string) {
	m.deviceID = d
}

func (m *Message) Fill() {
	if m.At == nil {
		t := time.Now()
		m.At = &t
	}

	if m.Tags == nil {
		m.Tags = make(map[string]string)
	}

	m.Tags["deviceID"] = m.deviceID
}
