CC := gcc

CFLAGS := -O3 \
          -I./OSMesa \

LDFLAGS := $(shell sdl2-config --libs) \
	    -lSDL2 \
	    -lm \
	    lib/libosmesa.a

SRCS := main.c
TARGET := sss3d-linux

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)