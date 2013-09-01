CFLAGS += -Wall -Werror -pedantic

all: bin/test

clean:
	rm src/*.o bin/*

src/errors.o: src/errors.c
src/buffer.o: src/buffer.c
src/header.o: src/header.c
src/payload.o: src/payload.c
src/message.o: src/message.c
src/parser.o: src/parser.c
src/test.o: src/test.c

bin/test: src/errors.o src/buffer.o src/header.o src/payload.o src/message.o src/parser.o src/test.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o bin/test src/errors.o src/buffer.o src/header.o src/payload.o src/message.o src/parser.o src/test.o