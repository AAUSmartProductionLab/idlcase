package transport

import (
	"encoding/json"
	"fmt"
	"log"
	"strings"
	"time"

	"bitbucket.org/ragroup/idlcase/dash/sensor"
	mqtt "github.com/eclipse/paho.mqtt.golang"
)

type Handler func(sensor.Message)

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

	// fields should now contain 0 => idl, 1 => deviceid, 2 => name
	if len(fields) != 3 {
		log.Printf("unknown mqtt topic format: %s", msg.Topic())
		return
	}

	smsg := sensor.Message{}
	smsg.DeviceID = fields[1]
	smsg.Kind = fields[2]
	smsg.At = time.Now()

	err := json.Unmarshal(msg.Payload(), &smsg)
	if err != nil {
		log.Printf("unable to unmarshal json from mqtt message: %s", err)
		return
	}

	for _, h := range s.Handlers {
		go h(smsg)
	}
}
