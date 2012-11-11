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
#include "socks_reply.hh"

std::string socks_reply::create(type type,
				int command,
				unsigned short port,
				unsigned long address){
	
	switch(type){
		case SOCKS4:{
			char buff[8];
			buff[0]=SOCKS4_REPLY_VERSION;
			buff[1]=command;
			memcpy(buff+2,&port,2);
			//buff[2]=port;
			//buff[3]=(port>>8);
			//buff[4]=(address>>24);
			//buff[5]=(address>>16);
			//buff[6]=(address>>8);
			//buff[7]=address;
			//buff[4]=address;
			//buff[5]=(address>>8);
			//buff[6]=(address>>16);
			//buff[7]=(address>>24);
			memcpy(buff+4,&address,4);
			return std::string(buff,8);
		}
			break;
		case SOCKS5:{
			char buff[10];
			buff[0]=SOCKS5_REPLY_VERSION;
			buff[1]=command;
			buff[2]=0x00;
			buff[3]=SOCKS5_ADDRESS_IPV4;//address_type;
			
			memcpy(buff+4,&address,4);
			
			//buff[4]=address;
			//buff[5]=(address>>8);
			//buff[6]=(address>>16);
			//buff[7]=(address>>24);

			//buff[4]=(address>>24);
			//buff[5]=(address>>16);
			//buff[6]=(address>>8);
			//buff[7]=address;
			
			//buff[8]=port;
			//buff[9]=(port>>8);
			
			memcpy(buff+8,&port,2);
			return std::string(buff,10);
		}			
		case SOCKS5_AUTH:{
			char buff[2];
			buff[0]=SOCKS5_REPLY_VERSION;
			buff[1]=command;
			return std::string(buff,2);
		}
			break;
	}
	
	return std::string();
}
