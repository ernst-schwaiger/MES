CPP := g++

OPENSSL_INCLUDE 		:= $(shell pkg-config --variable=includedir openssl)
OPENSSL_LIB 			:= $(shell pkg-config --variable=libdir openssl)

CCOPTS := -ggdb3 -O0 -Wall -Wextra -Werror -Isrc -I$(OPENSSL_INCLUDE)
LINKOPTS := -L$(OPENSSL_LIB)
all: FWManager

FWManager: CertificateHandler.o main.o
	$(CPP) $(LINKOPTS) -o FWManager CertificateHandler.o main.o -lssl -lcrypto

CertificateHandler.o: src/CertificateHandler.cpp src/CertificateHandler.h
	$(CPP) -c $(CCOPTS) src/CertificateHandler.cpp -o CertificateHandler.o

main.o: src/main.cpp src/CertificateHandler.h
	$(CPP) -c $(CCOPTS) src/main.cpp -o main.o

clean:
	rm -f CertificateHandler.o main.o FWManager
