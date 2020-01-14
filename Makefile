ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

clean:
	echo "hello"

.PHONY: playbook
playbook:
	docker run -w /project -e HOME=/project --rm -it -v $(ROOT_DIR)/ansible:/project \
		ansible/ansible-runner ansible-playbook main.yml -i inventory.yml