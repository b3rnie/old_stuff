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
#include "socks_request.hh"

bool socks_request::parse(const std::string &req,bool initial){	
	int pos=0;
	try{
		this->version=(unsigned char)req.at(pos++);
		
		if(4==version){
			this->command=(unsigned char)req.at(pos++);
			
			if((int)req.length()<pos+2){
				return false;
			}
			memcpy(&port,req.c_str()+pos,2);
						
			//this->port=(unsigned char)req.at(pos+1);
			//this->port<<=8;
			//this->port=port|(unsigned char)req.at(pos);
			pos+=2;
			
			if((int)req.length()<pos+4){
				return false;
			}
			
			memcpy(&address,req.c_str()+pos,4);
			pos+=4;
			
			//this->address=(unsigned char)req.at(pos+3);
			//this->address<<=8;
			//this->address=address|(unsigned char)req.at(pos+2);
			//this->address<<=8;
			//this->address=address|(unsigned char)req.at(pos+1);
			//this->address<<=8;
			//this->address=address|(unsigned char)req.at(pos);
			//pos+=4;
			
			while(req.at(pos)){
				this->socks4_user+=req.at(pos++);
				if(pos>=256)
					throw exception("huge user field");
			}
		}else if(5==version && initial){
			int nmethods=req.at(pos++);
			for(int i=0;i<nmethods;i++){
				socks5_auth_methods.push_back(req.at(pos++));
				if(i>=256)
					throw exception("huge number of methods");
			}
		}else if(5==version){
			this->command=(unsigned char)req.at(pos++);
			this->socks5_reserved=(unsigned char)req.at(pos++);
			this->socks5_address_type=(unsigned char)req.at(pos++);
			
			switch(socks5_address_type){
				case SOCKS5_ADDRESS_IPV4:
					//address=(unsigned char)req.at(pos+3);
					//address<<=8;
					//address=address|(unsigned char)req.at(pos+2);
					//address<<=8;
					//address=address|(unsigned char)req.at(pos+1);
					//address<<=8;
					//address=address|(unsigned char)req.at(pos);
					//pos+=4;
					//pos+=2;
					
					if((int)req.length()<pos+4){
						return false;
					}
					
					memcpy(&address,req.c_str()+pos,4);
					pos+=4;
					break;
				case SOCKS5_ADDRESS_DOMAINNAME:{
					unsigned char bytes=req.at(pos++);
					
					for(int i=0;i<bytes;i++){
						//char ch;
						pos++;
					}
				}
					break;
				case SOCKS5_ADDRESS_IPV6:{
					pos+=16;
				}
					break;
				default:
					throw exception("non existant address type");
			}
			
			//memcpy(&port,req.c_str()+pos,2);
			if((int)req.length()<pos+2){
				return false;
			}
			memcpy(&port,req.c_str()+pos,2);
			//this->port=(unsigned char)req.at(pos+1);
			//this->port<<=8;
			//this->port=port|(unsigned char)req.at(pos);
			pos+=2;
		
		}else{
			throw exception("strange version");
		}
		
	}catch(...){
		return false;
	}
	
	return true;
}

