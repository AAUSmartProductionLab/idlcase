package api

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"

	_ "github.com/influxdata/influxdb1-client" // this is important because of the bug in go mod
	client "github.com/influxdata/influxdb1-client/v2"
)

type Measurement struct {
	Value float64 `json:"value"`
	Time  string  `json:"time"`

	Tags map[string]string `json:"tags"`
}

// CurrentValues is a HTTPHandler which outputs all current sensor values
type CurrentValues struct {
	Client client.Client
}

func (c *CurrentValues) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodGet {
		http.Error(
			w,
			fmt.Sprintf("wrong http method: %s, only %s allowed", r.Method, http.MethodGet),
			http.StatusBadRequest,
		)
		return
	}

	// fetch available measurements
	query := client.NewQuery("show measurements;", "idl", "")
	result, err := c.Client.Query(query)
	if err != nil {
		err = fmt.Errorf("unable to query influxdb: %w", err)
		log.Printf(err.Error())
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	measurements := make([]string, 0, 10)
	for _, row := range result.Results[0].Series[0].Values {
		m, _ := row[0].(string)
		measurements = append(measurements, m)
	}

	// fetch all last values from each measurement
	series := make(map[string][]Measurement)
	for _, m := range measurements {
		data := make([]Measurement, 0)

		query := client.NewQuery(
			fmt.Sprintf("SELECT last(*) FROM %s GROUP BY *;", m),
			"idl",
			"",
		)

		response, err := c.Client.Query(query)
		if err != nil {
			err = fmt.Errorf("unable get response from influxdb: %w", err)
			log.Printf(err.Error())
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		for _, row := range response.Results[0].Series {
			f, _ := row.Values[0][1].(json.Number).Float64()
			entry := Measurement{
				Value: f,
				Time:  row.Values[0][0].(string),
				Tags:  row.Tags,
			}

			data = append(data, entry)
		}
		series[m] = data
	}

	// encode as json
	enc := json.NewEncoder(w)
	err = enc.Encode(series)
	if err != nil {
		log.Printf("unable to encode json to http writer: %s", err)
	}
}
