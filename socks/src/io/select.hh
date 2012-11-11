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
extern "C"{
#ifdef __OpenBSD__ /* probably applies to more systems */
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#else
#include <sys/select.h> /* new POSIX standard */
#endif
}

//#include <iostream>
#include <vector>
#include <algorithm>
#include <cerrno>

#include "file_descriptor.hh"

#ifndef __SELECT_HH
#define __SELECT_HH

namespace io{	
	/**
	 * Monitor a set of filedescriptors for changes.
	 *
	 * The select class is used to monitor readability/writeability
	 * on a set of filedescriptors. It's basically implemented as a 
	 * wrapper around the select syscall.
	 */
	class select{
	private:
		/* private data */
		std::vector<io::file_descriptor *> file_descriptors_read;
		std::vector<io::file_descriptor *> file_descriptors_write;
		fd_set read_set;
		fd_set write_set;
		
		/* private methods */
		bool contains(const std::vector<io::file_descriptor *> &what,
			      io::file_descriptor *fd)const;
	public:
		
		select();

		void add_read(io::file_descriptor *fd);
		void remove_read(io::file_descriptor *fd);
		
		void add_write(io::file_descriptor *fd);
		void remove_write(io::file_descriptor *fd);
		
		
		bool write_contains(io::file_descriptor *fd)const;
		bool read_contains(io::file_descriptor *fd)const;

		
		bool can_read(io::file_descriptor *fd)const;
		bool can_write(io::file_descriptor *fd)const;
		
		/// Try to select for |seconds| seconds.
		bool try_select(const int seconds){
			struct timeval tv;
			tv.tv_sec=seconds;
			tv.tv_usec=0;
			return try_select(tv);
		}
		
		/// Try to select for |tv| length.
		bool try_select(struct timeval tv);
		
	        class exception : public base_exception{
		public:
			exception(const std::string &what) :
				base_exception("io::select "+what){};
		};
	};
}



#endif
