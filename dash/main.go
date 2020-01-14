package main

import (
	"fmt"
	"log"
	"time"

	"github.com/fatih/color"
	"github.com/jroimartin/gocui"
	"github.com/shirou/gopsutil/net"
)

var (
	statusView    *gocui.View
	mqttConnected *gocui.View
	logView       *gocui.View
	g             *gocui.Gui
)

func main() {
	var err error
	g, err = gocui.NewGui(gocui.OutputNormal)
	if err != nil {
		log.Panicln(err)
	}
	defer g.Close()

	g.SetManagerFunc(layout)

	if err := g.SetKeybinding("", gocui.KeyCtrlC, gocui.ModNone, quit); err != nil {
		log.Panicln(err)
	}

	go uiLoop()

	if err := g.MainLoop(); err != nil && err != gocui.ErrQuit {
		log.Panicln(err)
	}

}

func uiLoop() {
	for {
		refreshStatus()
		time.Sleep(time.Millisecond * 250)
	}
}

func refreshStatus() {
	ifStat, err := net.Interfaces()
	if err != nil {
		panic(err)
	}

	// g.Update ensures no other routines
	// are updating the interface
	g.Update(func(g *gocui.Gui) error {
		statusView.Clear()
		for _, device := range ifStat {
			// We dont fancy localhost
			if device.Name == "lo" {
				continue
			}

			// We dont care about devices without addresses
			if len(device.Addrs) == 0 {
				continue
			}

			color.New(color.FgYellow).Fprintf(statusView, "%s\n", device.Name)
			for _, address := range device.Addrs {
				fmt.Fprintf(statusView, "%s\n", address.Addr)
			}
		}
		return nil
	})
}

func layout(g *gocui.Gui) error {
	maxX, maxY := g.Size()
	var err error
	if mqttConnected, err = g.SetView("mqtt", 0, 0, maxX-35, maxY/2-1); err != nil {
		if err != gocui.ErrUnknownView {
			return err
		}

		mqttConnected.Wrap = false
		mqttConnected.Title = "MQTT clients"
	}

	if statusView, err = g.SetView("stats", maxX-35, 0, maxX-1, maxY/2-1); err != nil {
		if err != gocui.ErrUnknownView {
			return err
		}

		statusView.Title = "Status"
	}

	if logView, err = g.SetView("log", 0, maxY/2-1, maxX-1, maxY); err != nil {
		if err != gocui.ErrUnknownView {
			return err
		}

		logView.Title = "Log"
		logView.Autoscroll = true
	}

	return nil
}

func quit(g *gocui.Gui, v *gocui.View) error {
	return gocui.ErrQuit
}
