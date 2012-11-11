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

#include "connection.hh"
#include "status.hh"
#include "config.hh"
#include "logfile.hh"
#include "io/select.hh"
#include "io/socket.hh"

class http_connection : public connection{
private:
	enum http_state{
		READING_REQUEST,
		SENDING_REPLY
	};
	
	http_state state;
	
	io::socket *client;
	
	std::string client_read_buff;
	std::string client_write_buff;

	bool reading_request(io::select &select);
	bool sending_reply(io::select &select);

	void clear_select(io::select &select);
	void create_reply();
public:
	http_connection(io::socket *client);
	~http_connection();
	
	bool work(io::select &select,struct timeval &tv);
	bool has_expired(io::select &select);
};
