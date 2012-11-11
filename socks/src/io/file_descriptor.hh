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
#include <cerrno>

extern "C"{
#include <unistd.h>
#include <fcntl.h>
}

#include "exception.hh"

#ifndef __FILE_DESCRIPTOR_HH
#define __FILE_DESCRIPTOR_HH

namespace io{
	
	/**
	 * This class encapsulates a file descriptor.
	 *
	 * The class is meant to be used as a baseclass for classes
	 * that has a file descriptor/socket.
	 */
	class file_descriptor{
	private:
		/// We dont want an instance to be copy-constructed.
		file_descriptor(const file_descriptor &rhs);
		/// ...
		file_descriptor &operator=(const file_descriptor &rhs);
		
	protected:
		/*
		 * The filedescriptor
		 */
		int fd;
	public:
		/**
		 * Default constructor.
		 * Initializes the file descriptor to -1.
		 */
		file_descriptor() : fd(-1){}
		
		/**
		 * Initialize the fd to fd.
		 */
		file_descriptor(int fd) : fd(fd){}
		
		/**
		 * Close the descriptor if it is "active".
		 */
		virtual ~file_descriptor(){
			if(fd!=-1)
				close(fd);
		}
		
		/**
		 * Set the descriptor to nonblocking state.
		 */
		void set_nonblocking();
		
		class exception : public base_exception{
		public:
			exception(const std::string &what) :
				base_exception("file_descriptor: "+what){};
		};
		
		/**
		 * Select is allowed to see the file descriptor.
		 */
		friend class select;
		
		/**
		 * ...
		 */
		friend bool operator==(const file_descriptor &lhs,
				       const file_descriptor &rhs){
			return lhs.fd==rhs.fd;
		}

		/**
		 * ...
		 */
		friend bool operator!=(const file_descriptor &lhs,
				       const file_descriptor &rhs){
			return !(lhs==rhs);
		}
	};
}

#endif

