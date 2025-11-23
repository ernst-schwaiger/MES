CC = gcc
CPP = g++

HOME_FOLDER 		:= $(shell echo ~)
OPENSSL_INCLUDE 	:= $(HOME_FOLDER)/openssl-local/include
OPENSSL_LIB 		:= $(HOME_FOLDER)/openssl-local/lib64
LIBP11_INCLUDE 		:= $(HOME_FOLDER)/libp11-local/include
LIBP11_LIB	 		:= $(HOME_FOLDER)/libp11-local/lib

CCOPTS = -O0 -ggdb3 -Wall -Wextra -Werror -I./src -I$(LIBP11_INCLUDE) -I$(OPENSSL_INCLUDE)
LINKOPTS= -L$(LIBP11_LIB) -L$(OPENSSL_LIB) -lp11 -lcrypto

signing-tool: main.o
	$(CPP) $(CCOPTS) main.o -o $@ $(LINKOPTS)

main.o: src/main.cpp src/P11Wrapper.h src/config.h
	$(CPP) -c $(CCOPTS) src/main.cpp -o $@

#
# Generates a header file containing 
# - the path to the PKCS shared object and 
# - the label of the signing key on the crypto token
#
src/config.h:
	echo "#pragma once" > $@
	echo "#define PKCS_MODULE \"$(PKCS_MODULE)\"" >> $@
	echo "#define TOKEN_SIGNING_LABEL \"$(TOKEN_SIGNING_LABEL)\"" >> $@

clean:
	rm -f signing-tool *.o