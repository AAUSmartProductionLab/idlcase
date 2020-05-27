package sensor

import (
	"encoding/json"
	"fmt"
	"time"

	"github.com/fatih/color"
	client "github.com/influxdata/influxdb1-client/v2"
)

type Data interface {
	Read() (map[string]string, map[string]interface{}, bool)
}

// Message represents a sensor message
type Message struct {
	DeviceID string // e.g. BEEEEF - extracted from mqtt topic
	Kind     string // e.g. temperature - extracted from mqtt topic
	At       time.Time

	Data
}

// UnmarshalJSON lets us handle our own unmarshalling
func (m *Message) UnmarshalJSON(b []byte) error {
	intermediate := struct{ Type *string }{}

	err := json.Unmarshal(b, &intermediate)
	if err != nil {
		return fmt.Errorf("unable to parse intermediate message: %s")
	}
	if intermediate.Type == nil {
		return fmt.Errorf("unable to determine message type, no type field found: %s", string(b))
	}

	switch *intermediate.Type {
	case "event":
		m.Data = &Event{}
	case "measurement":
		m.Data = &Measurement{}
	default:
		return fmt.Errorf("unknown message type: %s", intermediate.Type)
	}

	err = json.Unmarshal(b, m.Data)
	if err != nil {
		return fmt.Errorf("unable to parse %s message: %s", *intermediate.Type, err)
	}
	return nil
}

// Metric returns this metrics key
func (m *Message) Metric() string {
	return fmt.Sprintf("%s/%s", m.DeviceID, m.Kind)
}

// Since returns a human readable color formatted duration
func (m *Message) Since() string {
	since := time.Now().Sub(m.At).Round(time.Second)

	// If this is an event we should not set any colors
	// as we cannot know what kind of duration is bad or good
	if _, ok := m.Data.(*Event); ok {
		return since.String()
	}

	var sinceFormatted string

	switch {
	case since >= 10*time.Second:
		sinceFormatted = color.RedString("%s", since)
	case since >= 5*time.Second:
		sinceFormatted = color.YellowString("%s", since)
	default:
		sinceFormatted = since.String()
	}

	return sinceFormatted
}

func (m *Message) Points() (client.BatchPoints, error) {

	bp, err := client.NewBatchPoints(client.BatchPointsConfig{})
	if err != nil {
		return nil, fmt.Errorf("could not create new batch points: %w", err)
	}

	for {

		tags, values, ok := m.Data.Read()
		if !ok {
			break
		}

		tags["deviceId"] = m.DeviceID

		p, err := client.NewPoint(m.Kind, tags, values, m.At)
		if err != nil {
			return nil, fmt.Errorf("unable to create new influxdb point: %w", err)
		}

		bp.AddPoint(p)
	}

	return bp, nil
}
