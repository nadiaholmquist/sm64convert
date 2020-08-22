CC ?= gcc

all: sm64convert

sm64convert: sm64convert.c
	$(CC) -Og -g -I. -o $@ $<

clean:
	rm -f sm64convert sm64convert.exe
