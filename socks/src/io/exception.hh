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
#include <string>

#ifndef __IO_EXCEPTION_HH
#define __IO_EXCEPTION_HH

namespace io{
	/**
	 * Baseclass for exception types.
	 *
	 * The base_exception class is used as a baseclass for exceptions
	 * in the io package/namespace. It simplifies exception handling
	 * among other things.
	 */
	class base_exception{
	private:
		/// _what holds the exception description.
		std::string _what;
	public:
		/// Initialize exception description.
		base_exception(const std::string &what) : _what(what){};
		
		/// Return a copy of the exception description.
		std::string what()const{
			return _what;
		}
	};
}

#endif
