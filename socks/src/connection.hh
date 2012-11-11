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

//#include <iostream>
//#include <string>
//#include <vector>

extern "C"{
#include <time.h>
#include <sys/time.h>
}

#include "io/socket.hh"

#ifndef __CONNECTION_HH
#define __CONNECTION_HH

/**
 * One connection.
 */
class connection{
private:
	time_t last_active;
protected:
	
	static const int MAX_BUFF_SIZE=4096;
	
	void update_last_active(){
		last_active=time(NULL);
	}
	
	time_t get_last_active()const{return last_active;}
public:
	connection(){
		last_active=time(NULL);
	}
	
	virtual ~connection(){}
	
	virtual bool work(io::select &select,struct timeval &tv)=0;
	virtual bool has_expired(io::select &select){return false;}
};


#endif
