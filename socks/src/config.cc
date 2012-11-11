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
#include "config.hh"

config config::instance;

void config::load(const std::string &file){
	std::ifstream conf(file.c_str());
	
	if(!conf){
		throw exception(std::string("cant open: ")+file);
	}
	
	for(int i=0;i<(int)(sizeof(instance.speed)/sizeof(instance.speed[0]));i++){
		instance.speed[i].clear();
	}
	
	instance.accept_from.clear();
	instance.speed_port.clear();

	while(!conf.eof()){
		std::string line;
		getline(conf,line);
		
		
		if(line.find('#')!=std::string::npos)
			line.erase(line.find('#'),std::string::npos);
		
		int idx=line.find('=');
		if(idx==(int)std::string::npos){
			continue;
		}
		std::string option(line.substr(0,idx));
		std::string value(line.substr(idx+1,std::string::npos));
		
		/* not by me */
		value=value.substr(0,std::distance(std::find_if(value.rbegin(),value.rend(),
								std::not1(std::ptr_fun(isspace))),
						   option.rend()));
		value=value.substr(std::distance(value.begin(),std::find_if(value.begin(),value.end(),
									    std::not1(std::ptr_fun(isspace)))));
		
		
		std::cout<<"'"<<option<<"'  =  '"<<value<<"'"<<std::endl;
		
		if(option.find("speed")==0){
			parse_speed(option,value);
		}else if(option.find("listen interface")==0){
			instance.listen_interface=value;
		}else if(option.find("logfile")==0){
			instance.logfile=value;
		}else if(option.find("listen port")==0){
			instance.port=std::atol(value.c_str());
		}else if(option.find("run as")==0){
			instance.run_as=value;
		}else if(option.find("max clients")==0){
			instance.max_clients=std::atoi(value.c_str());
		}else if(option.find("accept from")==0){
			instance.accept_from.push_back(value);
		}else if(option.find("pipe timeout")==0){
			instance.pipe_timeout=std::atoi(value.c_str());
		}else if(option.find("http listen interface")==0){
			instance.http_listen_interface=value;
		}else if(option.find("http listen port")==0){
			instance.http_port=std::atol(value.c_str());
		}else{
			throw exception(std::string("unknown option: '")+
					option+std::string("'"));
		}
	}
}

bool config::can_connect(std::string ip){
	for(int i=0;i<(int)instance.accept_from.size();i++){
		int flags=0;
		flags|=REG_EXTENDED;
		flags|=REG_NOSUB;
		
		regex_t regex;
		if(regcomp(&regex,instance.accept_from[i].c_str(),flags)){
			throw exception(std::string("bad regex: ")+
					instance.accept_from[i]);
		}
		
		regmatch_t *pmatch=NULL;
		size_t nmatch=0;
		int eflags=0;
		if(!regexec(&regex,ip.c_str(),nmatch,pmatch,eflags)){
			regfree(&regex);
			return true;
		}
		regfree(&regex);
        }
	return false;
}

void config::parse_speed(std::string option,std::string value){
	bool port=false;
	if(option.find("speed_port")==0)
		port=true;
	
	int idx1=option.find('[');
	int idx2=option.find(']');
	
	std::string day_s(option.substr(idx1+1,idx2-idx1-1));
	int day=std::atoi(day_s.c_str());
	
	int pos=0;
	for(;;){
		int index=value.find(',',pos);
		std::string part=value.substr(pos,index-pos);
		
		std::string from=part.substr(0,part.find('-'));
		std::string to=part.substr(part.find('-')+1,
					   part.find('=')-part.find('-')-1);
		std::string speed=part.substr(part.find('=')+1,std::string::npos);
		
		int from_hr;
		int from_min;
		if(from.find(".")!=std::string::npos){
			from_hr=std::atoi(from.substr(0,from.find(".")).c_str());
			from_min=std::atoi(from.substr(from.find(".")+1,std::string::npos).c_str());
		}else{
			from_hr=std::atoi(from.c_str());
			from_min=0;
		}
		
		int to_hr;
		int to_min;
		if(to.find(".")!=std::string::npos){
			to_hr=std::atoi(to.substr(0,to.find(".")).c_str());
			to_min=std::atoi(to.substr(to.find(".")+1,std::string::npos).c_str());
		}else{
			to_hr=std::atoi(to.c_str());
			to_min=0;
		}
		
		struct time_slice tm(from_hr,from_min,
				     to_hr,to_min,
				     std::strtoul(speed.c_str(),NULL,10));
		
		if(port){
			std::cout<<"adding time_slice to destination port "<<day<<": ";
			instance.speed_port[day].push_back(tm);
		}else{
			std::cout<<"adding time_slice to day "<<day<<": ";
			instance.speed[day].push_back(tm);
		}
		
		std::cout<<tm.from_hr<<"."<<tm.from_min<<" -> "<<tm.to_hr<<"."<<tm.to_min<<" ("<<tm.speed<<")"<<std::endl;
		//std::cout<<from<<std::endl;
		//std::cout<<to<<std::endl;
		
		if(index==(int)std::string::npos)
			break;
		pos=index+1;
	}
	std::cout<<std::endl;
}

unsigned long int config::get_speed(const long port){
	time_t t=time(NULL);
	struct tm *now=localtime(&t);
	
	//std::cout<<"port: "<<port<<std::endl;
	
	unsigned long int speed=get_speed_time(instance.speed[now->tm_wday]);
	
	std::vector<struct time_slice> foo=instance.speed_port[port];
	if(foo.size())
		speed=get_speed_time(foo);
		
	return speed;
}

unsigned long int config::get_speed_time(const std::vector<struct time_slice> &day){
	unsigned long int speed=0;
	time_t t=time(NULL);
	
	struct tm *now=localtime(&t);
	for(int i=0;i<(int)day.size();i++){
		struct time_slice slice=day[i];
		
		int from=slice.from_hr*60+slice.from_min;
		int to=slice.to_hr*60+slice.to_min;
		
		int tm_now=now->tm_hour*60+now->tm_min;
		
		//std::cout<<from<<" -> "<<to<<" ("<<tm_now<<")"<<std::endl;
		if(tm_now>=from && tm_now<to){
			speed=slice.speed;
			break;
		}
	}
	
	return speed;
}

