package sensor

// Event represents an event happening
type Event struct {
	// Payload is used for machine readable content
	Payload string

	// Msg is a more human readable content
	Msg string
}

// Store saves this event into a record
func (e *Event) Read() (map[string]string, map[string]interface{}, bool) {
	return make(map[string]string), map[string]interface{}{
		"payload": e.Payload,
		"msg":     e.Msg,
	}, false
}
