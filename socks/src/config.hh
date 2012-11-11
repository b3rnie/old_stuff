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
#include <fstream>
#include <cerrno>
#include <algorithm>
#include <functional>
#include <map>

extern "C"{
#include <sys/types.h>
#include <regex.h>
#include <time.h>
}

#include "exception.hh"

#ifndef __CONFIG_HH__
#define __CONFIG_HH__

class config{
private:
	struct time_slice{
		int from_hr;
		int from_min;
		
		int to_hr;
		int to_min;
		unsigned long int speed;
		time_slice(int from_hr,int from_min,
			   int to_hr,int to_min,
			   unsigned long int speed){
			this->from_hr=from_hr;
			this->from_min=from_min;
			this->to_hr=to_hr;
			this->to_min=to_min;
			this->speed=speed;
		}
	};
	
	static config instance;
	
	static void parse_speed(std::string option,std::string value);
	static unsigned long int get_speed_time(const std::vector<struct time_slice> &day);
	
	std::string config_file;
	
	/** Configuration data */
	long port;
	long http_port;
	
	int max_clients;
	int pipe_timeout;
	
	std::string listen_interface;
	std::string http_listen_interface;
	
	std::string logfile;
	std::string run_as;
	
	std::vector<struct time_slice> speed[7];
	std::map<long,std::vector<struct time_slice> > speed_port;
	
	std::vector<std::string> accept_from;
public:
	static void load(const std::string &file);
	static config get_instance(){
		return instance;
	};
	
 	static int get_max_clients(){return instance.max_clients;}
	static std::string get_logfile(){return instance.logfile;}
	static std::string get_run_as(){return instance.run_as;}
	static long get_port(){return instance.port;}
 	static long get_http_port(){return instance.http_port;}
	//static std::string get_accept_from(){return instance.accept_from;}
	static std::string get_listen_interface(){return instance.listen_interface;}
	static std::string get_http_listen_interface(){return instance.http_listen_interface;}
	static unsigned long int get_speed(const long port);
	static int get_pipe_timeout(){return instance.pipe_timeout;}
	static bool can_connect(std::string ip);
	
	class exception : public base_exception{
	public:
		exception(const std::string &what) :
			base_exception("config: "+what){};
	};
};

#endif
