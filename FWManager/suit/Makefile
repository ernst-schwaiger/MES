APPLICATION = suit_gen_manifest

.PHONY: all create-manifest-basis generate-manifest clean

all:  create-manifest-basis generate-manifest clean

create-manifest-basis:
	python3 $(CURDIR)/gen_manifest.py $(CURDIR)/payload.bin
generate-manifest:
	python3 $(CURDIR)/suit-manifest-generator/bin/suit-tool create -i $(CURDIR)/out.json -o test.json -f json

clean:
	rm -rf $(CURDIR)/suit-manifest-generator/suit_tool/__pycache__