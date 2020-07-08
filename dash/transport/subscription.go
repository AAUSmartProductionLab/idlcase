package transport

import (
	"encoding/json"
	"fmt"
	"log"
	"strings"

	"bitbucket.org/ragroup/idlcase/dash/sensor"
	mqtt "github.com/eclipse/paho.mqtt.golang"
	influx "github.com/influxdata/influxdb1-client/v2"
)

type Handler func(Message)

type Message interface {
	SetDeviceID(string)
	DeviceID() string

	Metric() string
	Since() string

	UIValue() string
	UIUnit() string

	Point() (*influx.Point, error)
}

type Subscription struct {
	// usually a wildcard, matching multiple sensors
	Topic    string
	Handlers []Handler
}

// Run subscribes to topic and handles data
func (s *Subscription) Run() error {
	// subscribe - wait - if error stop
	if token := client.Subscribe(s.Topic, 0, s.handleMessage); token.Wait() && token.Error() != nil {
		return fmt.Errorf("unable to subscribe to topic: %w", token.Error())
	}

	return nil
}

func (s *Subscription) handleMessage(_ mqtt.Client, msg mqtt.Message) {
	fields := strings.FieldsFunc(msg.Topic(), func(r rune) bool {
		return r == '/'
	})

	// fields should now contain 0 => idl, 1 => deviceid, 2 => kind of message
	if len(fields) != 3 {
		log.Printf("unknown mqtt topic format: %s", msg.Topic())
		return
	}

	switch fields[2] {
	case "measurements":
		m := []*sensor.Measurement{}

		err := json.Unmarshal(msg.Payload(), &m)
		if err != nil {
			log.Printf("could not unmarshal: %s", err)
			return
		}

		for _, msg := range m {
			msg.SetDeviceID(fields[1])
			msg.Fill()
			for _, h := range s.Handlers {
				go h(msg)
			}
		}

	case "events":
		m := []*sensor.Event{}

		err := json.Unmarshal(msg.Payload(), &m)
		if err != nil {
			log.Printf("could not unmarshal: %s", err)
			return
		}

		for _, msg := range m {
			msg.SetDeviceID(fields[1])
			msg.Fill()
			for _, h := range s.Handlers {
				go h(msg)
			}
		}

	default:
		log.Printf("unknown message type: %s", fields[2])
		return
	}

}
