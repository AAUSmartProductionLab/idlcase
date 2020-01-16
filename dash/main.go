package main

import (
	"log"

	"github.com/jroimartin/gocui"
)

var (
	statusView    *Status
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
	layout(g)

	if err := g.SetKeybinding("", gocui.KeyCtrlC, gocui.ModNone, quit); err != nil {
		log.Panicln(err)
	}

	go statusView.Loop()

	if err := g.MainLoop(); err != nil && err != gocui.ErrQuit {
		log.Panicln(err)
	}

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

	s, err := g.SetView("stats", maxX-35, 0, maxX-1, maxY/2-1)
	if err != nil {
		if err != gocui.ErrUnknownView {
			return err
		}
		statusView = &Status{View: s}
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
