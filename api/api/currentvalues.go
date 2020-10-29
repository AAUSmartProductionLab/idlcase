package api

import (
	"fmt"
	"net/http"

	influxdb2 "github.com/influxdata/influxdb-client-go/v2"
)

// CurrentValues is a HTTPHandler which outputs all current sensor values
type CurrentValues struct {
	Client influxdb2.Client
}

func (c *CurrentValues) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(
			w,
			fmt.Sprintf("wrong http method: %s, only %s allowed", r.Method, http.MethodPost),
			http.StatusBadRequest,
		)
		return
	}

}
