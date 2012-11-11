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
#include "socks_connection.hh"

socks_connection::socks_connection(io::socket *client){
	this->client=client;
	this->server=NULL;
	this->listen=NULL;
	
	suspended=false;
	suspended_bytes_piped=0;
	suspended_to=0;
	state=READING_INITIAL_REQUEST;
}

socks_connection::~socks_connection(){
	status::remove(this);
	delete client;
	delete server;
	delete listen;
}

bool socks_connection::work(io::select &select,struct timeval &tv){
	try{
		switch(state){
			case READING_INITIAL_REQUEST:
				return reading_initial_request(select);
			case SOCKS4_CONNECTING_SERVER:
				return socks4_connecting_server(select);
			case WRITING_RESPONSE_SUCCESS:
				return writing_response_success(select);
			case WRITING_RESPONSE_FAILURE:
				return writing_response_failure(select);
			case SOCKS5_WRITING_AUTH_RESPONSE_SUCCESS:
				return socks5_writing_auth_response_success(select);
			case SOCKS5_CONNECTING_SERVER:
				return socks5_connecting_server(select);
			case SOCKS5_READING_REQUEST:
				return socks5_reading_request(select);
			case SOCKS4_WAITING_CONNECTION:
				return socks4_waiting_connection(select);
			case SOCKS5_WAITING_CONNECTION:
				return socks5_waiting_connection(select);
			case SOCKS4_WRITING_BIND_RESPONSE:
				return socks4_writing_bind_response(select);
			case SOCKS5_WRITING_BIND_RESPONSE:
				return socks5_writing_bind_response(select);
			case PIPEING:
				return pipeing(select,tv);
			default:
				logfile::log_error("CONNECTION HAS ILLEGAL STATE");
		}
	}catch(const base_exception &e){
		logfile::log_error(e.what());
	}catch(const io::base_exception &e){
		logfile::log_error(e.what());
	}catch(...){
		logfile::log_error("UNKNOWN ERROR!");
	}

	clear_select(select);

	return false;
}

bool socks_connection::has_expired(io::select &select){
	if(get_last_active()+config::get_pipe_timeout()<time(NULL)){
		clear_select(select);
		logfile::log_notice("*** CONNECTION EXPIRED ***");
		return true;
	}
	return false;
}

void socks_connection::clear_select(io::select &select)const{
	if(client){
		select.remove_read(client);
		select.remove_write(client);
	}
	
	if(server){
		select.remove_read(server);
		select.remove_write(server);
	}
	
	if(listen){
		select.remove_read(listen);
		select.remove_write(listen);
	}
}

bool socks_connection::reading_initial_request(io::select &select){
	if(select.can_read(client)){
		char buff[MAX_BUFF_SIZE];
		
		int bytes_read=client->read(buff,MAX_BUFF_SIZE);
		
		/* client dropped connection */
		if(!bytes_read){
			clear_select(select);
			return false;
		}
		
		/* just a EAGAIN error */
		if(bytes_read!=-1)
			client_read_buff+=std::string(buff,bytes_read);
		
		
		/* something strang, we just drop it */
		if((int)client_read_buff.length()>=MAX_BUFF_SIZE){
			logfile::log_notice("huge initial request, dropping it");
			clear_select(select);
			return false;
		}
		
		socks_request req;
		if(req.parse(client_read_buff,true)){
			/* request read and parsed successfully */
			
			select.remove_read(client);
			
			if(4==req.version){
				if(!can_connect()){
					socks_reply reply;
					client_write_buff=reply.create(socks_reply::SOCKS4,
								       socks_reply::SOCKS4_REPLY_REJECTED_OR_FAILED);
					state=WRITING_RESPONSE_FAILURE;
					select.add_write(client);

					logfile::log_notice(std::string("machine: ")+client->get_remote_address_dotted_quad()+
							    std::string(" cant connect"));
					return true;
				}
				
				switch(req.command){
					case socks_request::SOCKS4_REQUEST_COMMAND_CONNECT:
						try{
							//char buff[16];
							//unsigned long int addr=client->get_remote_address();
							//if(inet_ntop(AF_INET,&req.address,buff,sizeof(buff))==NULL){
							//	std::cout<<"CANT CONVERT"<<std::endl;
								//logfile::log_error("!UNABLE TO CONVERT ADDRESS!");
								//return false;
							//	}else{
							//	std::cout<<std::string(buff)<<std::endl;
							//	}
							/* ok to connect to? */
							server=new io::socket(req.address,req.port);
							set_status("connecting");
							if(server->try_connect_nonblock()){
								socks_reply reply;
								client_write_buff=reply.create(socks_reply::SOCKS4,
											       socks_reply::SOCKS4_REPLY_GRANTED,
											       server->get_remote_port(),
											       server->get_remote_address());
								state=WRITING_RESPONSE_SUCCESS;
								select.add_write(client);
							}else{
								state=SOCKS4_CONNECTING_SERVER;
								select.add_read(server);
								select.add_write(server);
							}
							
						}catch(...){
							socks_reply reply;
							client_write_buff=reply.create(socks_reply::SOCKS4,
										       socks_reply::SOCKS4_REPLY_REJECTED_OR_FAILED);
							state=WRITING_RESPONSE_FAILURE;
							
							select.add_write(client);
						}
						break;
					case socks_request::SOCKS4_REQUEST_COMMAND_BIND:
						logfile::log_notice("got a bind request");
						try{
							listen=new io::server_socket(0,1);
							socks_reply reply;
							client_write_buff=reply.create(socks_reply::SOCKS4,
										       socks_reply::SOCKS4_REPLY_GRANTED,
										       listen->get_port(),
										       listen->get_address());
							select.add_write(client);
							state=SOCKS4_WRITING_BIND_RESPONSE;
							//std::cout<<"port: "<<listen->get_port()<<std::endl;
							//std::cout<<"real port: "<<ntohs(listen->get_port())<<std::endl;
						}catch(...){
							socks_reply reply;
							client_write_buff=reply.create(socks_reply::SOCKS4,
										       socks_reply::SOCKS4_REPLY_REJECTED_OR_FAILED);
							state=WRITING_RESPONSE_FAILURE;
							select.add_write(client);
						}
						break;
						
					default:{
						logfile::log_notice("crap request");
						socks_reply reply;
						client_write_buff=reply.create(socks_reply::SOCKS4,
									       socks_reply::SOCKS4_REPLY_REJECTED_OR_FAILED);
						state=WRITING_RESPONSE_FAILURE;
						select.add_write(client);
						
					}
				}
			}else{
				socks_reply reply;
				client_write_buff=reply.create(socks_reply::SOCKS5_AUTH,
							       socks_reply::SOCKS5_AUTH_REPLY_NO_AUTH);
				state=SOCKS5_WRITING_AUTH_RESPONSE_SUCCESS;
				select.add_write(client);
			}
		}else{
			logfile::log_notice("request failed to parse (incomplete read?)");
		}
	}
	return true;
}

bool socks_connection::socks4_waiting_connection(io::select &select){
	if(select.can_read(listen) || select.can_write(listen)){
		server=listen->accept();
		if(server!=NULL){
			select.remove_read(listen);
			select.remove_write(listen);
			delete listen;
			listen=NULL;
			
			select.add_write(client);
			
			socks_reply reply;
			client_write_buff=reply.create(socks_reply::SOCKS4,
						       socks_reply::SOCKS4_REPLY_GRANTED);
			state=WRITING_RESPONSE_SUCCESS;
		}
	}
	return true;
}

bool socks_connection::socks5_waiting_connection(io::select &select){
	if(select.can_read(listen) || select.can_write(listen)){
		server=listen->accept();
		if(server!=NULL){
			select.remove_read(listen);
			select.remove_write(listen);
			delete listen;
			listen=NULL;
			
			select.add_write(client);
			
			socks_reply reply;
			client_write_buff=reply.create(socks_reply::SOCKS5,
						       socks_reply::SOCKS5_REPLY_SUCCESS);
			state=WRITING_RESPONSE_SUCCESS;
		}
	}
	return true;
}

bool socks_connection::socks4_writing_bind_response(io::select &select){
	if(select.can_write(client)){
		int bytes_written=client->write(client_write_buff.c_str(),
						client_write_buff.length());
				
		if(bytes_written!=-1){
			client_write_buff=client_write_buff.substr(bytes_written,
								   std::string::npos);
		}
		
		if(!client_write_buff.length()){
			select.remove_write(client);
			select.add_read(listen);
			select.add_write(listen);
			state=SOCKS4_WAITING_CONNECTION;
			
		}
	}
	return true;
}

bool socks_connection::socks5_writing_bind_response(io::select &select){
	if(select.can_write(client)){
		int bytes_written=client->write(client_write_buff.c_str(),
						client_write_buff.length());
		
		if(bytes_written!=-1){
			client_write_buff=client_write_buff.substr(bytes_written,
								   std::string::npos);
		}
		
		if(!client_write_buff.length()){
			select.remove_write(client);
			select.add_read(listen);
			select.add_write(listen);
			state=SOCKS5_WAITING_CONNECTION;
			
		}
	}
	return true;
}

bool socks_connection::socks4_connecting_server(io::select &select){
	if(select.can_read(server) || select.can_write(server)){
		select.remove_read(server);
		select.remove_write(server);
				
		socks_reply reply;
		if(server->is_connected()){
			//std::cout<<"connected to server"<<std::endl;
			client_write_buff=reply.create(socks_reply::SOCKS4,
						       socks_reply::SOCKS4_REPLY_GRANTED,
						       server->get_remote_port(),
						       server->get_remote_address());
					
			state=WRITING_RESPONSE_SUCCESS;
		}else{
			//std::cout<<"unable to connect to server"<<std::endl;
			client_write_buff=reply.create(socks_reply::SOCKS4,
						       socks_reply::SOCKS4_REPLY_REJECTED_OR_FAILED);
			
			state=WRITING_RESPONSE_FAILURE;
		}
		select.add_write(client); /* the answer */
	}
	return true;
}


bool socks_connection::writing_response_success(io::select &select){
	if(select.can_write(client)){
		int bytes_written=client->write(client_write_buff.c_str(),
						client_write_buff.length());
				
		if(bytes_written!=-1){
			client_write_buff=client_write_buff.substr(bytes_written,
								   std::string::npos);
		}
		
		if(!client_write_buff.length()){
			select.remove_write(client);
			
			client_write_buff=client_read_buff="";
			server_read_buff=server_write_buff="";

			select.add_read(client);
			select.add_read(server);
			state=PIPEING;
			
			set_status("pipeing");
		}
		
	}
	return true;
}

bool socks_connection::writing_response_failure(io::select &select){
	if(select.can_write(client)){
		int bytes_written=client->write(client_write_buff.c_str(),
						client_write_buff.length());
		
		if(bytes_written!=-1){
			client_write_buff=client_write_buff.substr(bytes_written,
								   std::string::npos);
			
		}
		
		/* all written? */
		if(!client_write_buff.length()){
			clear_select(select);
			return false;
		}
	}
	return true;
}

bool socks_connection::pipeing(io::select &select,struct timeval &tv){
	if(suspended){
		try_activate(select,tv);
		return true;
	}
	
		
	char buff[MAX_BUFF_SIZE];
	
	/* try to read from client */
	if(client!=NULL && server!=NULL && select.can_read(client)){
		int bytes_read=client->read(buff,MAX_BUFF_SIZE);
		
		/* client closed connection */
		if(0==bytes_read){
			select.remove_read(client);
			select.remove_write(client);
			delete client;
			client=NULL;
		}else if(-1!=bytes_read){
			server_write_buff+=std::string(buff,bytes_read);
			
			//if(server!=NULL && server_write_buff.length())
			select.add_write(server);
			
			update_last_active();
		}
	}
	
	/* try to read from server */
	if(server!=NULL && client!=NULL && select.can_read(server)){
		int bytes_read=server->read(buff,MAX_BUFF_SIZE);
		
		/* server closed connection */
		if(0==bytes_read){
			select.remove_read(server);
			select.remove_write(server);
			delete server;
			server=NULL;
		}else if(-1!=bytes_read){
			client_write_buff+=std::string(buff,bytes_read);
			
			if(client!=NULL && client_write_buff.length())
				select.add_write(client);
			
			update_last_active();
		}
	}
	
	/* try to write to client */
	if(client!=NULL && client_write_buff.length() && select.can_write(client)){
		int bytes_written=client->write(client_write_buff.c_str(),
						client_write_buff.length());
		
		if(bytes_written){
			client_write_buff=client_write_buff.substr(bytes_written,
								   std::string::npos);
			if(!client_write_buff.length())
				select.remove_write(client);
			
			suspended_bytes_piped+=bytes_written;
			
			update_last_active();
		}
	}
	
	/* try to write to server */
	if(server!=NULL && server_write_buff.length() && select.can_write(server)){
		int bytes_written=server->write(server_write_buff.c_str(),
						server_write_buff.length());
		
		if(bytes_written){
			server_write_buff=server_write_buff.substr(bytes_written,
								   std::string::npos);
			
			if(!server_write_buff.length())
				select.remove_write(server);
			
			
			suspended_bytes_piped+=bytes_written;
			
			update_last_active();
		}
	}
	
	
	
	if(client!=NULL){
		if((int)server_write_buff.length()>MAX_BUFF_SIZE)
			select.remove_read(client);
		else
			select.add_read(client);
	}else if(server!=NULL){
		select.remove_read(server);
	}
	
	if(server!=NULL){
		if((int)client_write_buff.length()>MAX_BUFF_SIZE)
			select.remove_read(server);
		else
			select.add_read(server);
	}else if(client!=NULL){
		select.remove_read(client);
	}
	
	if(server==NULL && server_write_buff.length()){
		logfile::log_notice("server closed connection before everything got written");
	}
	
	if(client==NULL && client_write_buff.length()){
		logfile::log_notice("client closed connection before everything got written");
	}
	
	
	if((server==NULL && !client_write_buff.length()) ||
	   (client==NULL && !server_write_buff.length())){
		//logfile::log_notice("pipe finished (type 1)");
		clear_select(select);
		return false;
	}
	
	if(server==NULL && client==NULL){
		logfile::log_notice("pipe finished (type 2)");
		clear_select(select);
		return false;
	}
	
	try_suspend(select,tv);
	
	return true;
}

bool socks_connection::socks5_writing_auth_response_success(io::select &select){
	if(select.can_write(client)){
		int bytes_written=client->write(client_write_buff.c_str(),
						client_write_buff.length());
		if(bytes_written!=-1){
			client_write_buff=client_write_buff.substr(bytes_written,
								   std::string::npos);
		}
		
		if(!client_write_buff.length()){
			select.remove_write(client);					
			select.add_read(client);
			
			client_read_buff="";
			state=SOCKS5_READING_REQUEST;
		}
	}
	return true;
}

bool socks_connection::socks5_reading_request(io::select &select){
	if(select.can_read(client)){
		char buff[MAX_BUFF_SIZE];
		int bytes_read=client->read(buff,MAX_BUFF_SIZE);
		
		if(!bytes_read){
			clear_select(select);
			return false;
		}
		
		if(bytes_read!=-1)
			client_read_buff+=std::string(buff,bytes_read);
		
		if((int)client_read_buff.length()>=MAX_BUFF_SIZE){
			logfile::log_notice("someone is owerflowing us!");
			clear_select(select);
			return false;
		}
		
		socks_request req;
		if(req.parse(client_read_buff)){
			/* request read and parsed successfully */
			select.remove_read(client);
			
			if(!can_connect()){
				socks_reply reply;
				client_write_buff=reply.create(socks_reply::SOCKS5,
							       socks_reply::SOCKS5_REPLY_CONNECTION_NOT_ALLOWED);
									
				state=WRITING_RESPONSE_FAILURE;
				select.add_write(client);
				
				logfile::log_notice(std::string("machine: ")+client->get_remote_address_dotted_quad()+
						    std::string(" cant connect"));
				
				return true;
			}
			
			
			switch(req.command){
				case socks_request::SOCKS5_REQUEST_COMMAND_CONNECT:
					try{
						//char buff[16];
						//unsigned long int addr=client->get_remote_address();
						//if(inet_ntop(AF_INET,&req.address,buff,sizeof(buff))==NULL){
						//	std::cout<<"CANT CONVERT"<<std::endl;
							//logfile::log_error("!UNABLE TO CONVERT ADDRESS!");
							//return false;
						//}else{
						//	std::cout<<"???"<<std::string(buff)<<std::endl;
						//	}

						//std::cout<<"socks5_request_command_connect"<<std::endl;
						/* ok to connect to? */
						server=new io::socket(req.address,req.port);
						//std::cout<<ntohs(req->port)<<std::endl;
						set_status("connecting");
						if(server->try_connect_nonblock()){
							socks_reply reply;
							client_write_buff=reply.create(socks_reply::SOCKS5,
										       socks_reply::SOCKS5_REPLY_SUCCESS,
										       server->get_remote_port(),
										       server->get_remote_address());
					
							state=WRITING_RESPONSE_SUCCESS;
							select.add_write(client);
						}else{
							state=SOCKS5_CONNECTING_SERVER;
							select.add_read(server);
							select.add_write(server);
						}
					}catch(...){
						socks_reply reply;
						client_write_buff=reply.create(socks_reply::SOCKS5,
									       socks_reply::SOCKS5_REPLY_HOST_UNREACHABLE);
						state=WRITING_RESPONSE_FAILURE;
						select.add_write(client);
					}
					break;
				case socks_request::SOCKS5_REQUEST_COMMAND_BIND:
					try{
						logfile::log_notice("socks5 bind request");
						listen=new io::server_socket();
						socks_reply reply;
						client_write_buff=reply.create(socks_reply::SOCKS5,
									       socks_reply::SOCKS5_REPLY_SUCCESS,
									       listen->get_port(),
									       listen->get_address());
						select.add_write(client);
						state=SOCKS5_WRITING_BIND_RESPONSE;
					}catch(...){
						socks_reply reply;
						client_write_buff=reply.create(socks_reply::SOCKS5,
									       socks_reply::SOCKS5_REPLY_GENERAL_SOCKS_OR_SERVER_FAILURE);
						state=WRITING_RESPONSE_FAILURE;
						select.add_write(client);
					}
					break;
				case socks_request::SOCKS5_REQUEST_COMMAND_UDP_ASSOCIATE:
				default:{
					logfile::log_notice("socks5 UDP associate request");
					socks_reply reply;
					client_write_buff=reply.create(socks_reply::SOCKS5,
								       socks_reply::SOCKS5_REPLY_GENERAL_SOCKS_OR_SERVER_FAILURE);
					state=WRITING_RESPONSE_FAILURE;
					select.add_write(client);
					
				}
			}
		}
	}
	
	return true;
}

bool socks_connection::socks5_connecting_server(io::select &select){
	if(select.can_read(server) || select.can_write(server)){
		select.remove_read(server); /* no more from server */
		select.remove_write(server);
		
		socks_reply reply;
		if(server->is_connected()){
			//std::cout<<"connected to server"<<std::endl;
			client_write_buff=reply.create(socks_reply::SOCKS5,
						       socks_reply::SOCKS5_REPLY_SUCCESS,
						       server->get_remote_port(),
						       server->get_remote_address());
					
			state=WRITING_RESPONSE_SUCCESS;
		}else{
			//std::cout<<"unable to connect to server"<<std::endl;
			
			client_write_buff=reply.create(socks_reply::SOCKS5,
						       socks_reply::SOCKS5_REPLY_HOST_UNREACHABLE);
			
			state=WRITING_RESPONSE_FAILURE;
		}
		select.add_write(client); /* the answer */
	}
	return true;
}

void socks_connection::try_activate(io::select &select,struct timeval &tv){
	if(!suspended)
		return;
	
	struct timeval tv2;
	gettimeofday(&tv2,NULL);
	unsigned long int time_now=tv2.tv_sec*1000+tv2.tv_usec/1000;
	
	if(suspended_to<=time_now){
		/*
		 * Active the connection. 
		 */
		suspended=false;
		if(suspended_descriptors_set[0])
			select.add_read(client);
		if(suspended_descriptors_set[1])
			select.add_write(client);
			
		if(suspended_descriptors_set[2])
			select.add_read(server);
		if(suspended_descriptors_set[3])
			select.add_write(server);
	}else{
		/*
		 * Can't activate it yet, just recalc the select time.
		 */
		unsigned long int select_time=suspended_to-time_now;
		
		tv2.tv_usec=(select_time%1000)*1000;
		tv2.tv_sec=(select_time-tv2.tv_usec/1000)/1000;
		
		if(tv2.tv_sec<tv.tv_sec){
			tv.tv_sec=tv2.tv_sec;
			tv.tv_usec=tv2.tv_usec;
		}else if(tv2.tv_sec==tv.tv_sec && tv.tv_usec>tv2.tv_usec){
			tv.tv_usec=tv2.tv_usec;
		}
	}
}

void socks_connection::try_suspend(io::select &select,struct timeval &tv){
	long remote_port=0;
	if(server)
		remote_port=server->get_remote_port_host_order();
	
	if(!config::get_speed(remote_port) || !suspended_bytes_piped)
		return;
	
	/* how long should it have taken? (in ms) */
	unsigned long int should_take=(unsigned long int)
		((((double)suspended_bytes_piped)/((double)config::get_speed(remote_port)))*1000.0);
	
	if(!should_take)
		return;
	
 	struct timeval tv2;
 	gettimeofday(&tv2,NULL);
	
	suspended_to=suspended_to+should_take;
	
	if(suspended_to<=(unsigned long int)(tv2.tv_sec*1000+tv2.tv_usec/1000)){
		/*
		 * just update suspend time.
		 */
		suspended_to=tv2.tv_sec*1000+tv2.tv_usec/1000;
		return;
	}
	
	/*
	 * calculate select time.
	 */
	unsigned long int sleep_time=suspended_to-tv2.tv_sec*1000-tv2.tv_usec/1000;
	
 	tv2.tv_usec=(sleep_time%1000)*1000;
 	tv2.tv_sec=(sleep_time-tv2.tv_usec/1000)/1000;
	
 	if(tv2.tv_sec<tv.tv_sec){
 		tv.tv_sec=tv2.tv_sec;
 		tv.tv_usec=tv2.tv_usec;
 	}else if(tv2.tv_sec==tv.tv_sec && tv.tv_usec>tv2.tv_usec){
 		tv.tv_usec=tv2.tv_usec;
 	}
	
	suspended=true;
 	suspended_bytes_piped=0;
 	/* remove filedescriptors... */
 	suspended_descriptors_set[0]=false;
 	suspended_descriptors_set[1]=false;
 	suspended_descriptors_set[2]=false;
 	suspended_descriptors_set[3]=false;
	
 	if(client!=NULL){
 		if(select.read_contains(client)){
 			select.remove_read(client);
 			suspended_descriptors_set[0]=true;
 		}
		
 		if(select.write_contains(client)){
 			select.remove_write(client);
 			suspended_descriptors_set[1]=true;
		}
	}

	if(server!=NULL){
		if(select.read_contains(server)){
			select.remove_read(server);
			suspended_descriptors_set[2]=true;
		}
		
		if(select.write_contains(server)){
			select.remove_write(server);
			suspended_descriptors_set[3]=true;
		}
	}
}

bool socks_connection::can_connect(){
	return config::can_connect(client->get_remote_address_dotted_quad());
}

void socks_connection::set_status(const std::string &what){
	std::string s=client->get_remote_address_dotted_quad()+"(";
	char buff[10];
	std::sprintf(buff,"%ld",client->get_remote_port_host_order());
	s+=std::string(buff)+") -> ";
	s+=server->get_remote_address_dotted_quad()+"(";
	std::sprintf(buff,"%ld",server->get_remote_port_host_order());
	s+=std::string(buff)+") ("+what+")";
	
	status::set_status(this,s);
}
