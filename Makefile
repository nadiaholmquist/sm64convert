all: sm64convert

sm64convert: sm64convert.c
	gcc -Og -g -I. -o $@ $<

clean:
	rm sm64convert
