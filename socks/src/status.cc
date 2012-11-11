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
#include "status.hh"

std::map<const connection *,std::string> status::s;

void status::remove(const connection *conn){
	s.erase(conn);
}

void status::set_status(const connection *conn,const std::string &what){
	s[conn]=what;
}

std::string status::get(){
	std::string str("number of connections: ");
	char buff[10];
	std::sprintf(buff,"%d",s.size());
	str+=buff;
	str+="<br><br>\n";
		
	std::map<const connection *,std::string>::const_iterator ci;
	for(ci=s.begin();ci!=s.end();++ci){
		str+=ci->second+"<br>\n";
	}
	return str;
}
