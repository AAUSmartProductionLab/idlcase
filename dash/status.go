package main

import (
	"fmt"
	"time"

	"github.com/fatih/color"
	"github.com/jroimartin/gocui"
	"github.com/shirou/gopsutil/net"
)

// Status view
type Status struct {
	*gocui.View
}

// Loop refreshes the status view
func (s *Status) Loop() {
	for {
		s.refreshStatus()
		time.Sleep(time.Millisecond * 250)
	}
}

func (s *Status) refreshStatus() {

	ifStat, err := net.Interfaces()
	if err != nil {
		panic(err)
	}

	// g.Update ensures no other routines
	// are updating the interface
	g.Update(func(g *gocui.Gui) error {
		s.Clear()

		for _, device := range ifStat {
			// We dont fancy localhost
			if device.Name == "lo" {
				continue
			}

			// We dont care about devices without addresses
			if len(device.Addrs) == 0 {
				continue
			}

			color.New(color.FgYellow).Fprintf(s, "%s\n", device.Name)
			for _, address := range device.Addrs {
				fmt.Fprintf(s, "%s\n", address.Addr)
			}
		}
		return nil
	})
}
