package sensor

type Event struct {
	// Payload is used for machine readable content
	Payload string

	// Msg is a more human readable content
	Msg string
}

// Store saves this event into a record
func (e *Event) Store(_ map[string]string, values map[string]interface{}) {
	values["payload"] = e.Payload
	values["msg"] = e.Msg
}
