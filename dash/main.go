package main

import (
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

	logStoreErrors := func(m sensor.Message) {
		err := store.Add(m)
		if err != nil {
			// panic is maybe quite overkill in this case - we should just
			// wait and see if its possible to reconnect
			panic(err)
		}

	}

	sub := transport.Subscription{
		Topic: "idl/#",
		Handlers: []transport.Handler{
			gui.SensorUpdate,
			logStoreErrors,
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
