package transport

import (
	"encoding/json"
	"fmt"
	"log"
	"strings"
	"time"

	mqtt "github.com/eclipse/paho.mqtt.golang"
	"github.com/fasmide/idlcase/dash/sensor"
)

type Handler func(sensor.Message)

type Subscription struct {
	// usually a wildcard, matching multiple sensors
	Topic    string
	Handlers []Handler

	client *mqtt.Client
}

// Run subscribes to topic and handles data
func (s *Subscription) Run() error {
	opts := mqtt.NewClientOptions()
	opts.AddBroker("tcp://172.27.23.173:1883")
	opts.SetClientID("dash")
	opts.SetDefaultPublishHandler(s.handleMessage)

	c := mqtt.NewClient(opts)
	s.client = &c

	// connect - wait - if error stop
	if token := c.Connect(); token.Wait() && token.Error() != nil {
		return fmt.Errorf("unable to connect to mqtt broker: %w", token.Error())
	}

	// subscribe - wait - if error stop
	if token := c.Subscribe(s.Topic, 0, nil); token.Wait() && token.Error() != nil {
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
	smsg.Type = fields[2]
	smsg.At = time.Now()

	err := json.Unmarshal(msg.Payload(), &smsg.Measurement)
	if err != nil {
		log.Printf("unable to unmarshal json from mqtt message: %s", err)
		return
	}

	for _, h := range s.Handlers {
		go h(smsg)
	}
}
