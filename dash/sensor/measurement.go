package sensor

// Measurement represents some measured data
type Measurement struct {
	PrecisionValue *int                          `json:"precision"`
	Values         map[string]map[string]float64 // map[unit]map[name]value

	reader chan struct {
		tags   map[string]string
		values map[string]interface{}
	}
}

func (m *Measurement) loop() {
	for unit, vals := range m.Values {
		tags := map[string]string{"unit": unit}
		for name, value := range vals {
			tags["name"] = name
			values := map[string]interface{}{"value": value}
			m.reader <- struct {
				tags   map[string]string
				values map[string]interface{}
			}{
				tags:   tags,
				values: values,
			}
		}
	}
	close(m.reader)
}

func (m *Measurement) Read() (map[string]string, map[string]interface{}, bool) {
	if m.reader == nil {
		m.reader = make(chan struct {
			tags   map[string]string
			values map[string]interface{}
		})
		go m.loop()
	}

	row, ok := <-m.reader
	return row.tags, row.values, ok
}
