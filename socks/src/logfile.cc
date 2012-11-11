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
#include "logfile.hh"

std::ofstream *logfile::logf=NULL;

void logfile::set_log(const std::string &file){
	if(logf!=NULL)
		delete logf;
	
	logf=new std::ofstream(file.c_str());
	
	if(!*logf)
		throw exception(std::string("unable to open for writing: ")+file);
}

void logfile::log_error(const std::string &what){
	
	*logf<<get_timestr()<<" ERROR: "<<what<<std::endl;
}

void logfile::log_notice(const std::string &what){
	*logf<<get_timestr()<<" NOTICE: "<<what<<std::endl;
}

void logfile::log_panic(const std::string &what){
	*logf<<get_timestr()<<" PANIC: "<<what<<std::endl;
}

std::string logfile::get_timestr(){
	time_t tm=time(NULL);
	char *p=ctime(&tm);
	return std::string(p,std::strlen(p)-1);
}

