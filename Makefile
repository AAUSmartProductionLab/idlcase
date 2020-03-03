ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
UID:=$(shell id -u)
clean:
	rm -f ansible/dash-arm-build dash/dash
	rm -f ansible/api-arm-build api/api

dash: dash/*.go
	docker run -it --rm \
		-v $(ROOT_DIR):/project \
		-e GOPATH=/project/.go \
		-e GOCACHE=/project/.go/cache \
		-e GOARCH=arm \
		-w /project/dash \
		-u $(UID) \
		golang:1.13.8 go build -o ../ansible/dash-arm-build .

api: api/*.go
	docker run -it --rm \
		-v $(ROOT_DIR):/project \
		-e GOPATH=/project/.go \
		-e GOCACHE=/project/.go/cache \
		-e GOARCH=arm \
		-w /project/api \
		-u $(UID) \
		golang:1.13.8 go build -o ../ansible/api-arm-build .

.PHONY: playbook
playbook: dash api
	docker run -w /project -e HOME=/project --rm -it \
		-v $(ROOT_DIR)/ansible:/project \
		-v $(ROOT_DIR)/known_hosts:/known_hosts \
		-v $(SSH_AUTH_SOCK):/ssh-agent \
		-e SSH_AUTH_SOCK=/ssh-agent \
		ansible/ansible-runner ansible-playbook main.yml $(OPTIONS)