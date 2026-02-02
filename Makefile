BINS=bounce thread
CFLAGS=-ggdb3
LIBS=-lglut -lglfw -l GL -lGLEW -lm -lpthread

all: $(BINS)

$(BINS): %: %.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
