objects = socks.o config.o logfile.o status.o socks_request.o socks_reply.o socks_connection.o http_connection.o
CC = g++
flags = -Wall -pedantic -g

foo:
	@echo ""
	@echo "please specify operating system"
	@echo ""
	@echo "make generic		generic"
	@echo "make solaris		solaris"
	@echo ""

generic:
	$(MAKE) socks
	$(CC) $(flags) -o socks $(objects) io/*.o
solaris:
	$(MAKE) socks
	$(CC) -lsocket -lnsl -lresolv -o socks $(objects) io/*.o
install:
	mkdir /usr/local/sbin
	mkdir /usr/local/etc
	cp socks /usr/local/sbin
	cp ../socks.conf /usr/local/etc
socks:	$(objects)
	cd io && $(MAKE)
socks.o: socks.cc *.hh io/*.hh
	$(CC) $(flags) -c socks.cc
config.o:	config.cc exception.hh
	$(CC) $(flags) -c config.cc
logfile.o:	logfile.cc logfile.hh exception.hh
	$(CC) $(flags) -c logfile.cc
status.o:	status.cc status.hh connection.hh exception.hh
	$(CC) $(flags) -c status.cc
socks_request.o:	socks_request.cc socks_request.hh exception.hh
	$(CC) $(flags) -c socks_request.cc
socks_reply.o:	socks_reply.cc socks_reply.hh exception.hh
	$(CC) $(flags) -c socks_reply.cc
socks_connection.o:	socks_connection.cc socks_connection.hh connection.hh io/*.hh
	$(CC) $(flags) -c socks_connection.cc
http_connection.o:	http_connection.cc http_connection.hh connection.hh io/*.hh
	$(CC) $(flags) -c http_connection.cc
clean:
	cd io && $(MAKE) clean
	rm socks $(objects)

