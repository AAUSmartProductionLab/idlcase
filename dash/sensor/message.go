package sensor

import (
	"encoding/json"
	"fmt"
	"time"

	"github.com/fatih/color"
	client "github.com/influxdata/influxdb1-client/v2"
)

type Data interface {
	PrettyValue() string
	PrettyUnit() string

	StoreTags(map[string]string)
	StoreValues(map[string]interface{})

	Compare(interface{}) (int, error)
}

// Message represents a sensor message
type Message struct {
	DeviceID string
	Type     string
	At       time.Time

	Data Data
}

// UnmarshalJSON lets us handle our own unmarshalling
func (m *Message) UnmarshalJSON(b []byte) error {
	if m.Type == "event" {
		// use event type
		m.Data = &Event{}
		return json.Unmarshal(b, &m.Data)
	}

	// all other values are considered measurements .. for now
	m.Data = &Measurement{}
	return json.Unmarshal(b, &m.Data)
}

// Metric returns this metrics key
func (m *Message) Metric() string {
	return fmt.Sprintf("%s/%s", m.DeviceID, m.Type)
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

func (m *Message) ToPoint() (*client.Point, error) {
	tags := map[string]string{
		"deviceID": m.DeviceID,
	}
	m.Data.StoreTags(tags)

	values := map[string]interface{}{}
	m.Data.StoreValues(values)

	p, err := client.NewPoint(m.Type, tags, values, m.At)
	if err != nil {
		return nil, fmt.Errorf("unable to create new influxdb point: %w", err)
	}

	return p, nil
}
