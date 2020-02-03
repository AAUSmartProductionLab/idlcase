package main

import (
	"log"

	"github.com/fasmide/idlcase/dash/gui"
	"github.com/fasmide/idlcase/dash/sensor"
	"github.com/fasmide/idlcase/dash/storage"
	"github.com/fasmide/idlcase/dash/transport"
)

func main() {

	store, err := storage.NewStore()
	if err != nil {
		panic(err)
	}

	logAndStore := func(m sensor.Message) {
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
