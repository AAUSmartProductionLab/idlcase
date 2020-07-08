package gui

import (
	"fmt"

	"bitbucket.org/ragroup/idlcase/dash/transport"
	"github.com/jroimartin/gocui"
)

var (
	statusView *Status
	sensorView *Sensor
	logView    *Log

	g *gocui.Gui
)

// Run initiates the GUI and blocks until finished
func Run() error {
	var err error
	g, err = gocui.NewGui(gocui.OutputNormal)
	if err != nil {
		return fmt.Errorf("unable to initialize gocui: %w", err)
	}

	defer g.Close()

	g.SetManagerFunc(layout)
	layout(g)

	if err := g.SetKeybinding("", gocui.KeyCtrlC, gocui.ModNone, quit); err != nil {
		return fmt.Errorf("unable to set keybinding: %w", err)
	}

	go statusView.Loop()
	go sensorView.Loop()

	if err := g.MainLoop(); err != nil && err != gocui.ErrQuit {
		return fmt.Errorf("main loop failed: %w", err)
	}

	return nil
}

// SensorUpdate is the glue that connects our sensorView to the mqtt transport
func SensorUpdate(msg transport.Message) {
	sensorView.Update(msg)
}

func layout(g *gocui.Gui) error {
	maxX, maxY := g.Size()

	sV, err := g.SetView("mqtt", 0, 0, maxX-35, maxY/2-1)
	if err != nil {
		if err != gocui.ErrUnknownView {
			return err
		}
		sensorView = &Sensor{View: sV}
		sensorView.Init()
	}

	s, err := g.SetView("stats", maxX-35, 0, maxX-1, maxY/2-1)
	if err != nil {
		if err != gocui.ErrUnknownView {
			return err
		}

		statusView = &Status{View: s}
		statusView.Title = "Status"
	}

	l, err := g.SetView("log", 0, maxY/2-1, maxX-1, maxY)
	if err != nil {
		if err != gocui.ErrUnknownView {
			return err
		}
		logView = &Log{View: l}
		logView.Init()
	}

	return nil
}

func quit(g *gocui.Gui, v *gocui.View) error {
	return gocui.ErrQuit
}
