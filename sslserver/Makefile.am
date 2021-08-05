AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS = server
server_SOURCES = src/sslserver.c\
		 inc/sslserver.h
INCLUDES = -I ./../inc
server_LDFLAGS = -L /usr/lib/x86_64-linux-gnu/
server_LDADD = -lssl -lcrypto -lpthread
