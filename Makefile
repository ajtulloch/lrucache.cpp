BUILD = build

all:
	make build
	make test


build:
	mkdir -p $(BUILD) && cd $(BUILD) && cmake ../ -G Ninja && ninja -v

test:
	$(BUILD)/lrucache

.PHONY: analysis build lint
