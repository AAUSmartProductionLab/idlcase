package main

import (
	"github.com/fasmide/idlcase/dash/gui"
	"github.com/fasmide/idlcase/dash/transport"
)

func main() {

	sub := transport.Subscription{Topic: "idl/#",
		Handlers: []transport.Handler{
			gui.SensorUpdate,
		},
	}

	err := sub.Run()
	if err != nil {
		panic(err)
	}

	// gui.Run blocks until the user exits
	err = gui.Run()
	if err != nil {
		panic(err)
	}
}
