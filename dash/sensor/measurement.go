package sensor

type Measurement struct {
	PrecisionValue *int                          `json:"precision"`
	Values         map[string]map[string]float64 // map[unit]map[field]value
}

func (m *Measurement) Store(tags map[string]string, values map[string]interface{}) {
	for unit, values := range m.Values {
		tags["unit"] = unit
		for field, value := range values {
			values[field] = value
		}
	}
}
