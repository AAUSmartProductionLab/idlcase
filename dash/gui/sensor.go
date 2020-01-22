package gui

import (
	"fmt"
	"sort"
	"sync"
	"time"

	"github.com/fasmide/idlcase/dash/sensor"

	"github.com/fatih/color"
	"github.com/jroimartin/gocui"
)

// Sensor handles sensor view logic
type Sensor struct {
	*gocui.View
	sync.RWMutex

	// metrics is sorted and used to ensure
	// the same print order
	metrics []string

	// lastMessages holds the last message received indexed by metric
	lastMessages map[string]DisplayMessage
}

// DisplayMessage knows about the previous value
// and decides if the color should be red green or neutral
type DisplayMessage struct {
	sensor.Message

	color *color.Color
}

func (s *Sensor) Init() {
	s.metrics = make([]string, 0)
	s.lastMessages = make(map[string]DisplayMessage)

	s.Wrap = false
	s.Title = "Sensors"
}

// Update handles sensor messages
func (s *Sensor) Update(msg sensor.Message) {
	s.Lock()

	lMsg, exists := s.lastMessages[msg.Metric()]
	dMsg := DisplayMessage{Message: msg}

	// decide a color for the metric value
	if lMsg.Measurement.Value == msg.Measurement.Value {
		dMsg.color = color.New(color.FgWhite)
	}
	if lMsg.Measurement.Value < msg.Measurement.Value {
		dMsg.color = color.New(color.FgGreen)
	}
	if lMsg.Measurement.Value > msg.Measurement.Value {
		dMsg.color = color.New(color.FgRed)
	}

	s.lastMessages[msg.Metric()] = dMsg

	if !exists {
		// if this is the first time we have seen this device
		// add it to metrics and have it sorted
		s.metrics = append(s.metrics, msg.Metric())
		sort.Strings(s.metrics)
	}

	s.Unlock()
	// render will do its thing eventually
}

// Loop ensures the view is updated at a fixed rate
func (s *Sensor) Loop() {
	for {
		s.render()
		time.Sleep(250 * time.Millisecond)
	}
}

func (s *Sensor) render() {
	g.Update(func(g *gocui.Gui) error {
		s.RLock()
		s.Clear()

		// loop deviceIDs as these are sorted correctly
		for _, idx := range s.metrics {
			lMsg := s.lastMessages[idx]
			precisionFormat := fmt.Sprintf("%%9.%df", lMsg.Precision())

			fmt.Fprintf(s, "%-18.18s: %s %-4s %s\n",
				lMsg.Metric(),
				lMsg.color.Sprintf(precisionFormat, lMsg.Measurement.Value),
				lMsg.Measurement.Unit,
				time.Now().Sub(lMsg.At),
			)
		}

		s.RUnlock()
		return nil
	})

}
