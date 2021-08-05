AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS = client
client_SOURCES = src/sslclient.c\
		 src/sm3.c\
		 inc/sslclient.h\
		 inc/sm3.h
INCLUDES = -I ./../inc
client_LDFLAGS = -L /usr/lib/x86_64-linux-gnu
client_LDADD = -lssl -lcrypto

