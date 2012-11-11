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
#include "connection.hh"

connection::connection(io::socket *client){
	this->client=client;
	this->listen=NULL;
	this->server=NULL;
	
	this->bytes_piped=0;
	this->is_suspended=false;
	gettimeofday(&this->bytes_piped_start,NULL);
		
	this->last_active=time(NULL);
	
	cs=READING_INITIAL_REQUEST;
	path.push_back(READING_INITIAL_REQUEST);
}

connection::~connection(){
	//std::cout<<"~connection"<<std::endl;
	delete client;
	delete server;
	delete listen;
}

void connection::set_state(state cs){
	//std::cout<<"setting state: "<<cs<<std::endl;
	path.push_back(cs);
	this->cs=cs;
}
	
connection::state connection::get_state()const{
	return cs;
}

void connection::clear_buffers(){
	client_read_buff=client_write_buff="";
	server_read_buff=server_write_buff="";
}

