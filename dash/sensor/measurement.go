package sensor

import "fmt"

type Measurement struct {
	PrecisionValue *int `json:"Precision"`
	Value          float64
	Unit           string
}

// Precision defaults to 2 decimals
func (m *Measurement) Precision() int {
	if m.PrecisionValue == nil {
		return 2
	}
	return *m.PrecisionValue
}

func (m *Measurement) PrettyUnit() string {
	return m.Unit
}

func (m *Measurement) PrettyValue() string {
	precisionFormat := fmt.Sprintf("%%9.%df", m.Precision())
	return fmt.Sprintf(precisionFormat, m.Value)
}

func (m *Measurement) StoreTags(h map[string]string) {
	h["unit"] = m.Unit
}

func (m *Measurement) StoreValues(h map[string]interface{}) {
	h["value"] = m.Value
}

func (m *Measurement) Compare(d interface{}) (int, error) {
	in, ok := d.(*Measurement)
	if !ok {
		return 0, fmt.Errorf("cannot compare %T to %T", d, m)
	}

	if in.Value == m.Value {
		return 0, nil
	}

	if in.Value > m.Value {
		return 1, nil
	}

	return -1, nil
}
