CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Werror
TARGET = ipkcpd
OBJS = ipkcpd.o
LIBS = -lpthread

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
