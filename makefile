CC=gcc
CFLAGS=-c -Wall -I/usr/local/include
LDFLAGS=-L/usr/local/lib -lfcgi -lcurl
SOURCES=$(wildcard src/*.c)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=binary/hados

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm $(EXECUTABLE) src/*.o

distclean: clean