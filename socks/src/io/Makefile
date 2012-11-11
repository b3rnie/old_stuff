objects = socket.o server_socket.o select.o file_descriptor.o
headers = socket.hh server_socket.hh select.hh exception.hh file_descriptor.hh
CC = g++
flags = -Wall -pedantic -g

all:	$(objects)
socket.o:	socket.cc $(headers)
	$(CC) $(flags) -c socket.cc
server_socket.o:	server_socket.cc $(headers)
	$(CC) $(flags) -c server_socket.cc
select.o:	select.cc $(headers)
	$(CC) $(flags) -c select.cc
file_descriptor.o:	file_descriptor.cc $(headers)
	$(CC) $(flags) -c file_descriptor.cc
clean:
	rm $(objects) 
