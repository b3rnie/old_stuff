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
#ifndef __SOCKET_HH
#define __SOCKET_HH

extern "C"{
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>
}


#include <iostream>
#include <string>

#include "file_descriptor.hh"
#include "select.hh"
#include "exception.hh"

namespace io{
	/**
	 * An client socket.
	 *
	 * The socket class is an endpoint for communication between
	 * two machines.
	 * The socket class is modeled after java.net.Socket.
	 */
	class socket : public io::file_descriptor{
	protected:
		
		/// Address to the host the socket is connected to.
		struct sockaddr_in serv;
		
		/// Remove SIGPIPE madness.
		void fix_sigpipe(){
			if(signal(SIGPIPE,SIG_IGN)==SIG_ERR){
				throw exception(std::string("signal failed: ")+
						std::string(strerror(errno)));
			}
		}
	public:
		/**
		 * Create a new socket from an existing filedescriptor and
		 * address. This constructor is used by |io::server_socket::accept()|.
		 */
 		socket(const int fd,const struct sockaddr_in serv);
		
		/**
		 * Create a new socket connected to |address| on port |port|.
		 * |address| is in network byte order.
		 * |port| is in network byte order.
		 */
		socket(unsigned long addr,unsigned short port);
		
		/**
		 * ...
		 */
		virtual ~socket(){};
		
		/**
		 * Try to connect the socket without blocking.
		 * If successful |try_connect_nonblock()| will return true.
		 */
	        bool try_connect_nonblock();
		
		/**
		 * If |try_connect_nonblock()| returns false you should monitor
		 * the socket for readability and call |is_connected()| to find
		 * out if the connection was successful.
		 */
		bool is_connected();
		
		/**
		 * Try to read up to |count| bytes from the socket and put them
		 * in |buff| with timeout |timeout|.
		 * 
		 * If |timeout|=0 read wont block (at all?).
		 *
		 * Return values:
		 * -1: read will block.
		 *  0: other end closed socket.
		 * >0: number of bytes read.
		 */
		ssize_t read(void *buff,size_t count,const int timeout=0);
		
		/**
		 * Try to write up to |count| bytes from |buff| to the socket with
		 * timeout |timeout|.
		 *
		 * If |timeout|=write wont block.
		 *
		 * |write()| returns the number of bytes written.
		 */
		ssize_t write(const void *buff,size_t count,const int timeout=0);
		
		/**
		 * Read exectly |count| bytes from socket to |buff| with
		 * timeout |timeout|.
		 */
		void read_all(void *buff,size_t count,const int timeout=0);
		
		/**
		 * Write exactly |count| bytes from |buff| to socket with
		 * timeout |timeout|.
		 */
		void write_all(const void *buff,size_t count,const int timeout=0);
		
		/// Return address of other end in network byte order.
		unsigned long get_remote_address()const{
			return serv.sin_addr.s_addr;
		}
		
		/// Return port of other end in network byte order.
		unsigned short get_remote_port()const{
			return serv.sin_port;
		}
		
		std::string get_remote_address_dotted_quad();
		long get_remote_port_host_order();
		
		/**
		 * ...
		 */
	        class exception : public base_exception{
		public:
			exception(const std::string &what) :
				base_exception("socket: "+what){};
		};
	};
}

#endif
