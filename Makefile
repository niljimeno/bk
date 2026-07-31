all:
	gcc main.c -o bk

run:
	make
	rm -rf result
	cp -r att2 result
	./bk att1 result

install:
	make
	sudo cp bk /usr/local/bin/bk
