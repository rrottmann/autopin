# inject a random key at compile time
CC = gcc -Dpepper=\"`head -c32 /dev/urandom | base64 -i | tr -d '\n'`\"

src = $(wildcard *.c)
obj = $(src:.c=.o)

LDFLAGS = -lbsd

getpin: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) getpin


install:
	install -t /usr/local/sbin -m 700 -o root -g root getpin
	install -t /usr/local/sbin -m 700 -o root -g root util/dice5
	install -t /usr/local/sbin -m 700 -o root -g root util/autopin
	install -t /usr/local/sbin -m 744 -o root -g root util/checkpin
	install -t /usr/local/sbin -m 744 -o root -g root util/forgetpin
	install -t /usr/local/sbin -m 744 -o root -g root util/gennonce
	install -t /usr/local/sbin -m 744 -o root -g root util/gensecret
	install -t /usr/local/sbin -m 744 -o root -g root util/getcardkeyid
	install -t /usr/local/sbin -m 744 -o root -g root util/getcardserial
	install -t /usr/local/sbin -m 744 -o root -g root util/forgetpin
	install -t /usr/local/sbin -m 700 -o root -g root util/pinentry-fake
	install -t /usr/local/sbin -m 700 -o root -g root util/reload-gpgagent
	install -t /usr/local/sbin -m 700 -o root -g root util/sealdata
	install -t /usr/local/sbin -m 700 -o root -g root util/unsealdata
