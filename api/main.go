package main

import (
	"fmt"
	"net/http"
	"os"

	"bitbucket.org/ragroup/idlcase/api/api"
	mqtt "github.com/eclipse/paho.mqtt.golang"
	influxdb2 "github.com/influxdata/influxdb-client-go/v2"
)

func main() {
	c, err := connect()
	if err != nil {
		panic(err)
	}

	s := &http.ServeMux{}
	s.Handle("/forward/", &api.MQTTForwarder{Client: c})

	// /values/current makes room for future /values/xyz endpoints
	s.Handle("/values/current/", &api.CurrentValues{Client: influxdb2.NewClient("http://localhost:8086", "")})

	err = http.ListenAndServe(":9090", s)

	fmt.Printf("http server failed: %s", err)
	os.Exit(1)
}

func connect() (mqtt.Client, error) {
	opts := mqtt.NewClientOptions()
	opts.AddBroker("tcp://127.0.0.1:1883")
	opts.SetClientID("api")

	client := mqtt.NewClient(opts)
	// connect - wait - if error stop
	if token := client.Connect(); token.Wait() && token.Error() != nil {
		return nil, fmt.Errorf("unable to connect to mqtt broker: %w", token.Error())
	}

	return client, nil
}

func dbConnect()
