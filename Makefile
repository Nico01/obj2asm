CC = clang
CFLAGS = -Wall -g #-Wextra

SOURCES  := $(wildcard *.c)
INCLUDES := $(wildcard *.h)
OBJECTS  := $(SOURCES:.c=.o)

LDFLAGS =

all: obj2asm

obj2asm: $(OBJECTS)
	${CC} ${CFLAGS} $^ -o $@ ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c $^ -o $@

.PHONY: clean
clean:
	rm -f *.o obj2asm
