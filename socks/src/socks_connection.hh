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
#include <iostream>

#include "config.hh"
#include "logfile.hh"
#include "status.hh"
#include "connection.hh"
#include "socks_request.hh"
#include "socks_reply.hh"
#include "io/socket.hh"
#include "io/server_socket.hh"
#include "io/select.hh"

#ifndef __SOCKS_CONNECTION_HH
#define __SOCKS_CONNECTION_HH

class socks_connection : public connection{
private:
	/**
	 * Connection states.
	 */
	enum connection_state{
		READING_INITIAL_REQUEST,
		WRITING_RESPONSE_FAILURE,
		WRITING_RESPONSE_SUCCESS,
		
		SOCKS4_WAITING_CONNECTION,
		SOCKS5_WAITING_CONNECTION,
		
		SOCKS4_WRITING_BIND_RESPONSE,
		SOCKS5_WRITING_BIND_RESPONSE,
		
		SOCKS4_CONNECTING_SERVER,
		
		SOCKS5_READING_REQUEST,
		SOCKS5_WRITING_AUTH_RESPONSE_SUCCESS,
		SOCKS5_CONNECTING_SERVER,
		
		PIPEING
	};
	
	
	io::socket *client;
	io::socket *server;
	io::server_socket *listen;
	
	connection_state state;
	
	std::string client_read_buff;
	std::string client_write_buff;
		
	std::string server_read_buff;
	std::string server_write_buff;
	
	
	bool suspended;
	unsigned long int suspended_to;
	unsigned long int suspended_bytes_piped;
	bool suspended_descriptors_set[4];
	
	bool reading_initial_request(io::select &select);
	bool socks4_connecting_server(io::select &select);
	bool socks5_connecting_server(io::select &select);
	bool socks5_reading_request(io::select &select);
	bool socks5_writing_auth_response_success(io::select &select);
	bool pipeing(io::select &select,struct timeval &tv);
	bool writing_response_failure(io::select &select);
	bool writing_response_success(io::select &select);
	bool socks4_waiting_connection(io::select &select);
        bool socks5_waiting_connection(io::select &select);
	
	bool socks4_writing_bind_response(io::select &select);
	bool socks5_writing_bind_response(io::select &select);

	socks_connection(const connection &rhs);
	socks_connection &operator=(const connection &rhs);
	
	bool can_connect();
	void set_status(const std::string &what);
	void clear_select(io::select &select)const;
	
	void try_suspend(io::select &select,struct timeval &tv);
	void try_activate(io::select &select,struct timeval &tv);
public:
	socks_connection(io::socket *client);
	~socks_connection();
	
	bool work(io::select &select,struct timeval &tv);
	bool has_expired(io::select &select);
};

#endif
