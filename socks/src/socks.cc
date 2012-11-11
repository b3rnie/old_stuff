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
#include <vector>
#include <string>

extern "C"{
#include <signal.h>
}

#include "config.hh"
#include "logfile.hh"
#include "exception.hh"
#include "connection.hh"
#include "socks_connection.hh"
#include "http_connection.hh"
#include "io/select.hh"
#include "io/socket.hh"
#include "io/server_socket.hh"

std::string config_file;

void sig_hup(int sig){
	logfile::log_notice("RELOADING CONFIGURATION");
	try{
		config::load(config_file);
	}catch(...){
		logfile::log_panic("!CANT RELOAD CONFIGURATION FILE!");
		exit(EXIT_FAILURE);
	}
	signal(SIGHUP,sig_hup);
	
}


int main(int argc,char *argv[]){
	if(argc!=2){
		std::cout<<"usage: socks <CONFIGFILE>"<<std::endl;
		exit(EXIT_FAILURE);
	}
	
	config_file=argv[1];
	
	io::server_socket *socks_sock=NULL;
	io::server_socket *http_sock=NULL;
	try{
		config::load(config_file);
		logfile::set_log(config::get_logfile());

		//std::cout<<config::get_http_listen_interface()<<std::endl;
		
		
		
		socks_sock=new io::server_socket(config::get_port(),20,config::get_listen_interface());
		http_sock=new io::server_socket(config::get_http_port(),20,config::get_http_listen_interface());
		
	}catch(const base_exception &e){
		std::cout<<e.what()<<std::endl;
		goto die;
	}catch(const io::base_exception &e){
		std::cout<<e.what()<<std::endl;
		goto die;
	}catch(...){
		std::cout<<"unknown exception in init code"<<std::endl;
		goto die;
	}	

	//std::cout<<"mupp"<<std::endl;
	signal(SIGHUP,sig_hup);
	
	try{
		io::select select;
		select.add_read(http_sock);
		select.add_read(socks_sock);
		
		logfile::log_notice("server started");
		
		struct timeval tv;
		tv.tv_sec=10;
		tv.tv_usec=0;
		
		/** main server loop */
		for(std::vector<connection *> connections;;){
			//std::cout<<"selecting for "<<tv.tv_sec<<" seconds, "<<tv.tv_usec<<" ns."<<std::endl;
			
			if(!select.try_select(tv)){
				/*
				 * nothing ready.
				 * no point in looping over the connections
				 * and wasting CPU cycles.
				 */
				/*
				 * *update* we must loop over the connections to activate suspended... 
 				 */
			}
			
			tv.tv_sec=10;
			tv.tv_usec=0;
			
			if(select.can_read(socks_sock)){
				io::socket *new_sock=socks_sock->accept();
				
				if(new_sock!=NULL){
					//logfile::log_notice("adding new connection");
					connections.push_back(new socks_connection(new_sock));
					select.add_read(new_sock);
				}
			}
			
			if(select.can_read(http_sock)){
				
				io::socket *new_sock=http_sock->accept();
				
				if(new_sock!=NULL){
					connections.push_back(new http_connection(new_sock));
					select.add_read(new_sock);
				}
			}
			
			for(std::vector<connection *>::iterator j=connections.begin();
			    j!=connections.end();){
				if((*j)->has_expired(select) || !(*j)->work(select,tv)){
					delete *j;
					j=connections.erase(j);
				}else{
					++j;
				}
			}
		}
	}catch(const io::base_exception &e){
		logfile::log_panic(e.what());
	}catch(const base_exception &e){
		logfile::log_panic(e.what());
	}catch(...){
		logfile::log_panic("unknown exception caught");
	}	
 die:
 	delete socks_sock;
 	delete http_sock;
 	exit(EXIT_FAILURE);
}

