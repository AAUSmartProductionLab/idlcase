package transport

import (
	"fmt"

	mqtt "github.com/eclipse/paho.mqtt.golang"
)

var client mqtt.Client

func Connect() error {
	opts := mqtt.NewClientOptions()
	opts.AddBroker("tcp://127.0.0.1:1883")
	opts.SetClientID("dash")

	client = mqtt.NewClient(opts)
	// connect - wait - if error stop
	if token := client.Connect(); token.Wait() && token.Error() != nil {
		return fmt.Errorf("unable to connect to mqtt broker: %w", token.Error())
	}

	return nil
}

func PublishNoPayload(path string) error {
	token := client.Publish(path, 0, false, []byte{})
	token.Wait()
	return token.Error()
}
