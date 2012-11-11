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
#include <string>
#include <vector>

extern "C"{
#include <string.h>
}

#include "exception.hh"

#ifndef __SOCKS_REQUEST_HH
#define __SOCKS_REQUEST_HH

class socks_request{
public:
	const static int SOCKS4_REQUEST_COMMAND_CONNECT=0x01;
        const static int SOCKS4_REQUEST_COMMAND_BIND=0x02;
	
	const static int SOCKS5_REQUEST_COMMAND_CONNECT=0x01;
        const static int SOCKS5_REQUEST_COMMAND_BIND=0x02;
        const static int SOCKS5_REQUEST_COMMAND_UDP_ASSOCIATE=0x03;
	
	const static int SOCKS5_ADDRESS_IPV4=0x01;
        const static int SOCKS5_ADDRESS_DOMAINNAME=0x03;
        const static int SOCKS5_ADDRESS_IPV6=0x04;
	
	unsigned char version;
	unsigned char command;	
	unsigned char socks5_reserved;
	unsigned char socks5_address_type;	
	unsigned short port;
	unsigned long address;
	std::string socks4_user;
	
	std::vector<int> socks5_auth_methods;
	
	bool parse(const std::string &req,bool initial=false);
	
	class exception : public base_exception{
	public:
		exception(const std::string &what) :
			base_exception("socks_request: "+what){}
	};
};

#endif

