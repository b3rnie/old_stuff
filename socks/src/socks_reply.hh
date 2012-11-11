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
#include <string>

#ifndef __SOCKS_REPLY_HH
#define __SOCKS_REPLY_HH

/**
 * ...
 */
class socks_reply{
public:
	/// Reply type
	enum type{
		SOCKS4,
		SOCKS5,
		SOCKS5_AUTH
	};
	
	const static int SOCKS4_REPLY_VERSION=0;
	
	const static int SOCKS4_REPLY_GRANTED=90;
	const static int SOCKS4_REPLY_REJECTED_OR_FAILED=91;
	const static int SOCKS4_REPLY_REJECTED_IDENTD_DOWN=92;	
	const static int SOCKS4_REPLY_REJECTED_IDENTD_DIFFER=93;
	
	const static int SOCKS5_REPLY_VERSION=5;
	
	const static int SOCKS5_REPLY_SUCCESS=0x00;
	const static int SOCKS5_REPLY_GENERAL_SOCKS_OR_SERVER_FAILURE=0x01;
	const static int SOCKS5_REPLY_CONNECTION_NOT_ALLOWED=0x02;
	const static int SOCKS5_REPLY_NETWORK_UNREACHABLE=0x03;
	const static int SOCKS5_REPLY_HOST_UNREACHABLE=0x04;
	const static int SOCKS5_REPLY_CONNECTION_REFUSED=0x05;
	const static int SOCKS5_REPLY_TTL_EXPIRED=0x06;
	const static int SOCKS5_REPLY_COMMAND_NOT_SUPPORTED=0x07;
	const static int SOCKS5_REPLY_ADDRESS_TYPE_NOT_SUPPORTED=0x08;
	
	const static int SOCKS5_AUTH_REPLY_NO_AUTH=0;
	const static int SOCKS5_AUTH_REPLY_GSSAPI=1;
	const static int SOCKS5_AUTH_REPLY_USERNAME_PASSWORD=2;
	const static int SOCKS5_AUTH_REPLY_NO_ACCEPTABLE=0xFF;
	
	const static int SOCKS5_ADDRESS_IPV4=0x01;
        const static int SOCKS5_ADDRESS_DOMAINNAME=0x03;
        const static int SOCKS5_ADDRESS_IPV6=0x04;

	
	/**
	 * Generate a valid socks reply.
	 */
	std::string create(type type,
			   int command,
			   unsigned short port=0,
			   unsigned long address=0);
	
};

#endif
