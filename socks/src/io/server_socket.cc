/* Copyright (C) 2003 <Bjorn Jensen-Urstad>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 * USA
 */
#include "server_socket.hh"

io::server_socket::server_socket(const long port,const int backlog,const std::string address){
	memset(&servaddr,'\0',sizeof(servaddr));
	
	servaddr.sin_family=AF_INET;
	if(address.size()){
		if(inet_pton(AF_INET,address.c_str(),&servaddr.sin_addr)<=0){
			throw exception(std::string("failed to convert address: ")+
					std::string(strerror(errno)));
		}
	}else{
		servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	}
	servaddr.sin_port=htons(port);
	
	
	if((fd=::socket(AF_INET,SOCK_STREAM,0))<0){
		throw exception(std::string("socket failed: ")+
				std::string(strerror(errno)));
	}
	
	set_nonblocking();
	
	int foo=1;
	int bar=sizeof(foo);
	if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&foo,bar)<0){
		throw exception(std::string("setsockopt failed: ")+
				std::string(strerror(errno)));
	}
	
	if((bind(fd,(struct sockaddr *)&servaddr,sizeof(servaddr)))<0){
		throw exception(std::string("bind failed: ")+
				std::string(strerror(errno)));
	}
	
	memset(&servaddr,'\0',sizeof(servaddr));
	size_t len=sizeof(struct sockaddr_in);
	if((getsockname(fd,(struct sockaddr *)&servaddr,&len))<0){
		throw exception(std::string("getsockname failed: ")+
				std::string(strerror(errno)));
	}
	
	//std::cout<<servaddr.sin_port<<std::endl;
	
	if((listen(fd,backlog))<0){
		throw exception(std::string("listen failed: ")+
				std::string(strerror(errno)));
	}
}

io::server_socket::~server_socket(){
	//std::cout<<"closing serversocket"<<std::endl;
}

io::socket *io::server_socket::accept(){
	
	struct sockaddr_in cliaddr;
	
	int cli_fd;
	socklen_t clilen=sizeof(cliaddr);
	
        MAIN:for(;;){
		cli_fd=::accept(fd,(struct sockaddr *)&cliaddr,&clilen);
		
		if(cli_fd<0){
			if(EINTR==errno)
				goto MAIN;
			if(EAGAIN==errno || EWOULDBLOCK==errno)
				return NULL;
			throw exception(std::string("accept: failed: ")+
					std::string(strerror(errno)));
		}else{
			break;
		}
	}
	
	return new io::socket(cli_fd,cliaddr);
}

