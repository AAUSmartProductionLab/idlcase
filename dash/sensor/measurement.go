package sensor

import (
	"encoding/json"
)

type Measurement struct {
	PrecisionValue *int                          `json:"precision"`
	Values         map[string]map[string]float64 // map[unit]map[name]value

	reader chan struct {
		tags   map[string]string
		values map[string]interface{}
	}
}

func (m *Measurement) UnmarshalJSON(b []byte) error {
	err = json.Unmarshal(b, m)
	if err != nil {
		return err
	}
	return nil
}

func (m *Measurement) Read() (map[string]string, map[string]interface{}, bool) {
	row, ok := <-m.reader
	return row.tags, row.values, ok
}
