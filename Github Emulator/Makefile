all: WTFserver WTF test

WTFserver: WTFserver.c 
	gcc WTFserver.c header.c -pthread -lcrypto -o WTFserver

WTF: WTF.c
	gcc WTF.c -lcrypto -o WTF

test:
	gcc test.c -o WTFtest

clean:
	rm -f WTFserver WTF WTFtest