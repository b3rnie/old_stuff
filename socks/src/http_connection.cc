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
#include "http_connection.hh"

http_connection::http_connection(io::socket *client){
	this->client=client;
	state=READING_REQUEST;
}

http_connection::~http_connection(){
	status::remove(this);
	delete client;
}

bool http_connection::work(io::select &select,struct timeval &tv){
	try{
		switch(state){
			case READING_REQUEST:
				return reading_request(select);
			case SENDING_REPLY:
				return sending_reply(select);
		}
		
	}catch(...){
		
	}
	clear_select(select);
	return false;
}

void http_connection::clear_select(io::select &select){
	if(client){
		select.remove_read(client);
		select.remove_write(client);
	}
}

bool http_connection::has_expired(io::select &select){
	if(get_last_active()+config::get_pipe_timeout()<time(NULL)){
		clear_select(select);
		status::remove(this);
		logfile::log_notice("*** HTTP CONNECTION EXPIRED ***");
		return true;
	}
	return false;
}

bool http_connection::reading_request(io::select &select){
	if(select.can_read(client)){
		char buff[MAX_BUFF_SIZE];
		int bytes_read=client->read(buff,MAX_BUFF_SIZE);
		
		if(!bytes_read){
			select.remove_read(client);
			return false;
		}
		
		if(bytes_read!=-1){
			client_read_buff+=std::string(buff,bytes_read);
		}
		
		if((int)client_read_buff.length()>=MAX_BUFF_SIZE){
			select.remove_read(client);
			return false;
		}

		if(client_read_buff.find("\r\n\r\n")!=std::string::npos ||
		   client_read_buff.find("\n\n")!=std::string::npos){
			select.remove_read(client);
			select.add_write(client);
			
			create_reply();
			state=SENDING_REPLY;
		}
	}
	return true;
}

bool http_connection::sending_reply(io::select &select){
	if(select.can_write(client)){
		int bytes_written=client->write(client_write_buff.c_str(),client_write_buff.length());
		
		client_write_buff=client_write_buff.substr(bytes_written,std::string::npos);
		
		if(!client_write_buff.length()){
			select.remove_write(client);
			return false;
		}
	}
	return true;
}

void http_connection::create_reply(){
	client_write_buff="HTTP/1.0 200 OK\r\n";
	client_write_buff+="Connection: close\r\n";
	client_write_buff+="Content-Type: text/html\r\n\r\n";
	client_write_buff+="<html>\n<head><title></title></head>\n";
	client_write_buff+="<body>\n";
	client_write_buff+=status::get();
	client_write_buff+="<hr>\nAuthor: Björn Jensen-Urstad ";
	client_write_buff+="<a href=mailto:bju@stacken.kth.se>bju@stacken.kth.se</a>\n</body>\n</html>\n";
}
