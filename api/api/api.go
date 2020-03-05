package api

import (
	"fmt"
	"io"
	"io/ioutil"
	"net/http"
	"path"

	mqtt "github.com/eclipse/paho.mqtt.golang"
)

type MQTTForwarder struct {
	Client mqtt.Client
}

func (m *MQTTForwarder) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(
			w,
			fmt.Sprintf("wrong http method: %s, only %s allowed", r.Method, http.MethodPost),
			http.StatusBadRequest,
		)
		return
	}

	rest, t := path.Split(r.URL.EscapedPath())
	if t == "" {
		http.Error(
			w,
			fmt.Sprintf("wrong url scheme: %s", r.URL.EscapedPath()),
			http.StatusBadRequest,
		)
		return
	}

	deviceID := path.Base(rest)
	if deviceID == "" {
		http.Error(
			w,
			fmt.Sprintf("wrong url scheme: %s", rest),
			http.StatusBadRequest,
		)
		return
	}

	payload, err := ioutil.ReadAll(
		io.LimitReader(r.Body, 1024*512),
	)
	if err != nil {
		http.Error(
			w,
			fmt.Sprintf("unable to read body: %s", err),
			http.StatusInternalServerError,
		)
		return
	}

	// with t and deviceID, we should now be able to produce a valid MQTT topic
	topic := path.Join("idl", deviceID, t)

	token := m.Client.Publish(topic, 0x0, false, payload)

	token.Wait()
	if token.Error() != nil {
		http.Error(
			w,
			fmt.Sprintf("could not forward data: %s", token.Error()),
			http.StatusInternalServerError,
		)
		return
	}

	fmt.Fprint(w, "Thanks a lot")
}
