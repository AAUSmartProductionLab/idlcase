package gui

import (
	"fmt"
	"sort"
	"sync"
	"time"

	"github.com/fasmide/idlcase/dash/sensor"
	"github.com/jroimartin/gocui"
)

// Sensor handles sensor view logic
type Sensor struct {
	*gocui.View
	sync.RWMutex

	// deviceNames is sorted and used to ensure
	// the same print order
	deviceIDs []string

	// lastMessages holds the last message received indexed by deviceID
	lastMessages map[string]sensor.Message
}

func (s *Sensor) Init() {
	s.deviceIDs = make([]string, 0)
	s.lastMessages = make(map[string]sensor.Message)

	s.Wrap = false
	s.Title = "Sensors"
}

// Update handles sensor messages
func (s *Sensor) Update(msg sensor.Message) {
	s.Lock()

	_, exists := s.lastMessages[msg.DeviceID]
	s.lastMessages[msg.DeviceID] = msg

	if !exists {
		// if this is the first time we have seen this device
		// add it to deviceIDs and have it sorted
		s.deviceIDs = append(s.deviceIDs, msg.DeviceID)
		sort.Strings(s.deviceIDs)
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
		for _, idx := range s.deviceIDs {
			lMsg := s.lastMessages[idx]
			fmt.Fprintf(s, "%s: %02f %s %s\n",
				lMsg.DeviceID,
				lMsg.Measurement.Value,
				lMsg.Measurement.Unit,
				time.Now().Sub(lMsg.At),
			)
		}

		s.RUnlock()
		return nil
	})

}
