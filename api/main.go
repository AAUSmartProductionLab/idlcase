package main

import (
	"fmt"
	"net/http"
	"os"

	"bitbucket.org/ragroup/idlcase/api/api"
	mqtt "github.com/eclipse/paho.mqtt.golang"
	_ "github.com/influxdata/influxdb1-client" // this is important because of the bug in go mod
	client "github.com/influxdata/influxdb1-client/v2"
)

func main() {
	c, err := connect()
	if err != nil {
		panic(err)
	}

	s := &http.ServeMux{}
	s.Handle("/forward/", &api.MQTTForwarder{Client: c})

	influx, err := client.NewHTTPClient(client.HTTPConfig{
		Addr: "http://localhost:8086",
	})

	if err != nil {
		fmt.Println("Error creating InfluxDB Client: ", err.Error())
	}
	defer influx.Close()

	// /values/current makes room for future /values/xyz endpoints
	s.Handle("/values/current/", &api.CurrentValues{Client: influx})

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
