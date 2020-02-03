package sensor

import "fmt"

type Event struct {
	Type string
	Msg  string
}

func (e *Event) Precision() int {
	return 0
}

func (e *Event) PrettyUnit() string {
	return ""
}

func (e *Event) PrettyValue() string {
	return e.Type
}

func (e *Event) StoreTags(h map[string]string) {
	h["type"] = e.Type
}

func (e *Event) StoreValues(h map[string]interface{}) {
	h["msg"] = e.Msg
}

func (e *Event) Compare(d interface{}) (int, error) {
	c, ok := d.(*Event)
	if !ok {
		return 0, fmt.Errorf("unable to compare %T to *Event", d)
	}

	if c.Msg == e.Msg && c.Type == e.Type {
		return 0, nil
	}

	// this seems rather stupid
	cs := fmt.Sprintf("%s%s", c.Type, c.Msg)
	ec := fmt.Sprintf("%s%s", e.Type, e.Msg)
	if cs < ec {
		return 1, nil
	}

	return -1, nil
}
