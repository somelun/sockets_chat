CC = clang
PORT = 1716
CFLAGS = -Wall -Wextra -O2 -DPORT=$(PORT)

# Targets
TARGETS = client server

# Source and object files
CLIENT_SRC = client.c
SERVER_SRC = server.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
SERVER_OBJ = $(SERVER_SRC:.c=.o)

.PHONY: all clean

all: $(TARGETS)

client: $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

server: $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGETS) *.o
