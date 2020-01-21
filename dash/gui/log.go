package gui

import "github.com/jroimartin/gocui"

import "fmt"

// Log represents our logging view
type Log struct {
	*gocui.View

	incoming chan string
}

// Init initializes logView
func (l *Log) Init() {
	l.incoming = make(chan string)
	l.Title = "Log"
	l.Autoscroll = true
}

// Loop starts the Log view's loop
func (l *Log) Loop() {
	for {
		fmt.Fprint(l, <-l.incoming)
	}
}

// Log logs something to the display
// thread safe
func (l *Log) Log(s string) {
	l.incoming <- s
}
