package main

import (
	"fmt"
	"log"

	"bitbucket.org/ragroup/idlcase/dash/fota"
	"bitbucket.org/ragroup/idlcase/dash/gui"
	"bitbucket.org/ragroup/idlcase/dash/storage"
	"bitbucket.org/ragroup/idlcase/dash/transport"
)

func main() {
	// we start off by connecting to MQTT
	err := transport.Connect()
	if err != nil {
		panic(fmt.Sprintf("unable to connect to MQTT: %s", err))
	}

	// fota is our over-the-air update manager
	// Its just a webserver with a few endpoints
	fota := fota.Webserver{
		PublishFirmware: transport.PublishNoPayload,
		Database:        "database/",
		StorePath:       "storage/",
	}
	err = fota.Setup()
	if err != nil {
		panic(err)
	}

	// run ota in its own routine
	go func() {
		err := fota.ListenAndServe()
		if err != nil {
			panic(err)
		}
	}()

	store, err := storage.NewStore()
	if err != nil {
		panic(err)
	}

	logAndStore := func(m transport.Message) {
		err = store.Add(m)
		if err != nil {
			log.Printf("unable to store data: %s", err)
		}
	}

	sub := transport.Subscription{
		Topic: "idl/#",
		Handlers: []transport.Handler{
			gui.SensorUpdate,
			logAndStore,
		},
	}

	err = sub.Run()
	if err != nil {
		panic(err)
	}

	// gui.Run blocks until the user exits
	err = gui.Run()
	if err != nil {
		panic(err)
	}
}
