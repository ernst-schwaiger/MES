# cross-compiler, strip machine code from binary
CC			:= arm-none-eabi-gcc
OBJCPY		:= arm-none-eabi-objcopy
CCOPTS		:= \
 -mtune=cortex-m0plus \
 -march=armv6s-m \
 -ffunction-sections \
 -fdata-sections \
 -fshort-enums \
 -Os \
 -fwrapv \
 -fno-common \
 -fdata-sections \
 -fno-delete-null-pointer-checks \
 -fdiagnostics-color 

# Configure version and message of our app
MSG="Hello, World!"
VERSION="1.0.0"
SEQUENCE_NUMBER="42"
 
all: update.tgz

update.tgz:
	MSG=$(MSG) VERSION=$(VERSION) SEQUENCE_NUMBER=$(SEQUENCE_NUMBER) ./generator.sh update.c manifest.mf
	$(CC) $(CCOPTS) -c update.c -o update.o
	$(OBJCPY) -O binary --only-section=.hellofunc update.o update.bin
	tar -czf update.tgz update.bin manifest.mf

clean:
	rm -f update.* manifest.mf
