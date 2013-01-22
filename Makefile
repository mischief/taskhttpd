
CFLAGS= -I./libtask

LIBS=

HTTPD=taskhttpd
OBJECTS= main.o libtask/libtask.a

$(HTTPD): $(OBJECTS) $(LIBS)
	$(CC) $(CLFAGS) -o $@ $^

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

libtask/libtask.a:
	make -C libtask/

.PHONY: clean
clean:
	$(RM) $(HTTPD) *.o
	make -C libtask/ clean

