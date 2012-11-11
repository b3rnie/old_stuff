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
#include "select.hh"

io::select::select(){
	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
}

void io::select::add_read(io::file_descriptor *fd){
	if(!contains(file_descriptors_read,fd))	
		file_descriptors_read.push_back(fd);
}

void io::select::remove_read(io::file_descriptor *fd){
	if(contains(file_descriptors_read,fd))
		file_descriptors_read.erase(find(file_descriptors_read.begin(),
						 file_descriptors_read.end(),
						 fd));
}

void io::select::add_write(io::file_descriptor *fd){
	if(!contains(file_descriptors_write,fd))
		file_descriptors_write.push_back(fd);
}

void io::select::remove_write(io::file_descriptor *fd){
	if(contains(file_descriptors_write,fd))
		file_descriptors_write.erase(find(file_descriptors_write.begin(),
						  file_descriptors_write.end(),
						  fd));
}

bool io::select::write_contains(io::file_descriptor *fd)const{
	return contains(file_descriptors_write,fd);
}

bool io::select::read_contains(io::file_descriptor *fd)const{
	return contains(file_descriptors_read,fd);
}

bool io::select::contains(const std::vector<io::file_descriptor *> &what,
			  io::file_descriptor *fd)const{
	return(what.end()!=find(what.begin(),what.end(),fd));
}

bool io::select::can_read(io::file_descriptor *fd)const{
	if(FD_ISSET(fd->fd,&read_set))
		return true;
	return false;
}

bool io::select::can_write(io::file_descriptor *fd)const{
	if(FD_ISSET(fd->fd,&write_set))
		return true;
	return false;
}

bool io::select::try_select(struct timeval tv){
	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	
	int high_fd=-1;
	for(int i=0;i<(int)file_descriptors_read.size();i++){
		int fd=file_descriptors_read[i]->fd;
		
		FD_SET(fd,&read_set);
		high_fd=fd>high_fd?fd:high_fd;
	}
	
	for(int i=0;i<(int)file_descriptors_write.size();i++){
		int fd=file_descriptors_write[i]->fd;
		FD_SET(fd,&write_set);
		high_fd=fd>high_fd?fd:high_fd;
	}
	
	//struct timeval tv;
	//	
	//tv.tv_sec=timeout;
	//tv.tv_usec=0;
	
	MAIN:for(;;){
		int ret=::select(high_fd+1,&read_set,&write_set,NULL,&tv);
		
		if(!ret){
			return false;
		}else if(ret<0){
			if(errno==EINTR)
				goto MAIN;
			throw exception(std::string("select failed: ")+
					std::string(strerror(errno)));
		}else{
			break;
		}
	}
	
	return true;
}





