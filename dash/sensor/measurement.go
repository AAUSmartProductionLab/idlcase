package sensor

import (
	"fmt"

	client "github.com/influxdata/influxdb1-client/v2"
)

// Measurement represents some measured data
type Measurement struct {
	Message

	Name  string
	Unit  string
	Value float64
}

func (m *Measurement) Metric() string {
	return fmt.Sprintf("%s/%s/%s", m.deviceID, m.Table, m.Name)
}

func (m *Measurement) UIValue() string {
	return fmt.Sprintf("%f", m.Value)
}
func (m *Measurement) UIUnit() string {
	return m.Unit
}

func (m *Measurement) Point() (*client.Point, error) {
	tags := m.Tags
	tags["name"] = m.Name
	tags["unit"] = m.Unit

	point, err := client.NewPoint(m.Table, tags, map[string]interface{}{
		"value": m.Value,
	}, *m.At)
	if err != nil {
		return nil, fmt.Errorf("unable to create influxdb point: %s", err)
	}

	return point, nil
}
