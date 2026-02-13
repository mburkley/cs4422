BINS=llist-deadlock thread fork pipe
CFLAGS=-ggdb3
LIBS=-lm -lpthread

all: $(BINS)

$(BINS): %: %.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
