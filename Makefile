ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

clean:
	rm -f ansible/dash dash/dash

# This step should be dockerized at some point
dash: dash/*.go
	cd dash; GOARCH=arm go build -o ../ansible/dash-arm-build .

.PHONY: playbook
playbook: dash
	docker run -w /project -e HOME=/project --rm -it \
		-v $(ROOT_DIR)/ansible:/project \
		-v $(SSH_AUTH_SOCK):/ssh-agent \
		-e SSH_AUTH_SOCK=/ssh-agent \
		ansible/ansible-runner ansible-playbook main.yml $(OPTIONS)