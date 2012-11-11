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
#include "socket.hh"

/*
 * create a socket object from an existing fd.
 */
io::socket::socket(const int fd,const struct sockaddr_in serv)
	: file_descriptor(fd), serv(serv){
	
	fix_sigpipe();
	set_nonblocking();
}

io::socket::socket(unsigned long addr,unsigned short port)
	: file_descriptor(){
	fix_sigpipe();
	
	memset(&serv,0,sizeof(serv));
	serv.sin_family=AF_INET;
	
	/* already in correct format */
	serv.sin_addr.s_addr=addr;
	serv.sin_port=port;
	
        if((fd=::socket(AF_INET,SOCK_STREAM,0))<0){
		throw exception(std::string("socket failed: ")+
				std::string(strerror(errno)));
	}

	set_nonblocking();
}

bool io::socket::try_connect_nonblock(){	
        /* 
	 * nonblocking timed connect,
	 * copied from the book "Unix Network Programming".
	 *
	 */ 

	/* set socket_fd in nonblocking state */
	//set_nonblocking();
	
	switch(connect(fd,(struct sockaddr *)&serv,sizeof(serv))){
		case 0: /* connected, we are done */
			return true;
		case -1:
			if(errno!=EINPROGRESS)
				throw exception(std::string("connect failed: ")+
						std::string(strerror(errno)));
	}
	return false;
}

bool io::socket::is_connected(){
	
	/* select might by lying... */
	int err=0;
	int len=sizeof(int);
	if(getsockopt(fd,SOL_SOCKET,SO_ERROR,&err,(socklen_t  *)&len)<0)
		throw exception(std::string("getsockopt failed: ")+
				std::string(strerror(errno)));
	
	if(err){
		/*
		 * We should give a reason...
		 */
		return false;
	}
	return true;
}

ssize_t io::socket::read(void *buff,size_t count,const int timeout){	
	if(timeout){
		io::select select;
		select.add_read(this);
		if(!select.try_select(timeout))
			throw exception("read timeout");
	}
	
	int r;
	MAIN:for(;;){
		r=::read(fd,buff,count);
		
		if(r<0){
			if(EAGAIN==errno){ /* this should not happen */
				//throw exception("read will block");
				return -1;
			}
			
			if(EINTR==errno){
			        goto MAIN;
			}
			
			throw exception(std::string("read failed: ")+
					std::string(strerror(errno)));
		}else{
			break;
		}
	}
	
	return r;
}

ssize_t io::socket::write(const void *buff,size_t count,const int timeout){
	if(timeout){		
		io::select select;
		select.add_write(this);
		if(!select.try_select(timeout))
			throw exception("write timeout");
	}
	
	int w;
        MAIN:for(;;){
		w=::write(fd,buff,count);
		
		if(w<0){
			if(EAGAIN==errno){
				return 0;
			}
			
			if(EINTR==errno){
				goto MAIN;
			}
			
			throw exception(std::string("write failed: ")+
					std::string(strerror(errno)));
		}else{
			break;
		}
	}
	
	return w;
}

void io::socket::read_all(void *buff,size_t count,const int timeout){
	while(count){
		ssize_t r=read(buff,count,timeout);
		count-=r;
		char *p=(char *)buff;
		p+=r;
		buff=reinterpret_cast<void *>(p);
		if(0==r){
			/* we expected count bytes but got an
			 * eof before all was read */
			//throw exception("ReadAll: got EOF before reading everything");
			/* ... */
		}
	}
}

void io::socket::write_all(const void *buff,size_t count,const int timeout){
	while(count){
		ssize_t w=write(buff,count,timeout);
		count-=w;
		char *p=(char *)buff;
		p+=w;
		buff=reinterpret_cast<void *>(p);
	}
}


std::string io::socket::get_remote_address_dotted_quad(){
	char buff[16];
	
	if(inet_ntop(AF_INET,&serv.sin_addr.s_addr,buff,sizeof(buff))==NULL){
		throw exception(std::string("unable to convert address to dotted quad format: ")+
				std::string(strerror(errno)));
	}
	
	return std::string(buff);
}

long io::socket::get_remote_port_host_order(){
	return ntohs(serv.sin_port);
}
