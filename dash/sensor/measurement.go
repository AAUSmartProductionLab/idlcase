package sensor

import "fmt"

// Measurement represents some measured data
type Measurement struct {
	Message

	Name  string
	Unit  string
	Value float64
}

func (m *Measurement) Metric() string {
	return fmt.Sprintf("%s/%s", m.deviceID, m.Name)
}
