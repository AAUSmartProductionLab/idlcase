ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

clean:
	rm -f ansible/dash dash/dash

# This step should be dockerized at some point
dash: dash/*.go
	cd dash; go build -o ../ansible/dash .

.PHONY: playbook
playbook: dash
	docker run -w /project -e HOME=/project --rm -it -v $(ROOT_DIR)/ansible:/project \
		ansible/ansible-runner ansible-playbook main.yml -i inventory.yml