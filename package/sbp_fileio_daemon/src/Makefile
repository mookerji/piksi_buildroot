TARGET=sbp_fileio_daemon
SOURCES= \
	main.c \
	sbp_fileio.c

LIBS=-lczmq -lsbp -lsbp_zmq
CFLAGS=-std=gnu11

CROSS=

CC=$(CROSS)gcc

all:	$(TARGET)
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)

clean:
	rm -rf $(TARGET)
