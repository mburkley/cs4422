BINS=list-mutex llist-deadlock thread fork pipe maps
CFLAGS=-ggdb3
LIBS=-lm -lpthread

all: $(BINS)

$(BINS): %: %.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
