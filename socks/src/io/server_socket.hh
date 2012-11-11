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
#include "file_descriptor.hh"
#include "socket.hh"
#include "select.hh"
#include "exception.hh"


#ifndef __IO_SERVERSOCKET_HH__
#define __IO_SERVERSOCKET_HH__

namespace io{
	/**
	 * An implementation of server sockets.
	 *
	 * The server_socket class waits for request.
	 */
	class server_socket : public io::file_descriptor{
	private:
		/// Address bound by server_socket.
		struct sockaddr_in servaddr;
	public:
		/**
		 * 
		 *
		 */
		server_socket(const long port=0,const int backlog=1,const std::string address="");
		~server_socket();
		
		/**
		 * Accept a connection.
		 *
		 * Returns an |io::socket| or NULL if no sockets can be accepted. 
		 */
		io::socket *accept();
		
		/// Return port bound in network byte order.
		unsigned short get_port()const{
			return servaddr.sin_port;
		}
		
		/// Return address bound in network byte order.
		unsigned long get_address()const{
			return servaddr.sin_addr.s_addr;
		}
		
		class exception : public base_exception{
		public:
			exception(const std::string &what) :
				base_exception("serversocket: "+what){};
		};
	};
}

#endif

