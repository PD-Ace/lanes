CC=gcc
CFLAGS=  -g -lpthread -lgcrypt -lyaml
LDFLAGS=
SOURCES=main.c state.c util.c network.c crypto.c ring.c workers.c

EXECUTABLE=lanes

all: $(SOURCES) 
		$(CC) $(CFLAGS) -o $(EXECUTABLE) $(SOURCES)
