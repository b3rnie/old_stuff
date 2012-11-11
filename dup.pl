#!/usr/bin/perl -w
#
# Copyright (C) 1999-2001 Bjorn Jensen-Urstad <bju@stacken.kth.se>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
$|++;
# use strict;
use File::Find;
use Digest::MD5;

my %OPTIONS=(RENAME_DUPLICATE      => 0,
	     DELETE_DUPLICATE      => 0);

my $md5=Digest::MD5->new;
my $bytes_duplicated=0;
my %table;

die "usage: dup.pl [-r -d] <DIR>" unless @ARGV;

foreach my $arg(@ARGV){
	if($arg eq "-r"){
		$OPTIONS{RENAME_DUPLICATE}=1;
	}elsif($arg eq "-d"){
		$OPTIONS{DELETE_DUPLICATE}=1;
	}else{
		&update_table($arg, \%table)
	}
}

&check_table(\%table);
print int($bytes_duplicated/1024)," Kbytes duplicated\n";

sub update_table{
	my($dir, $table) = @_;
	find({follow => 1,
	      wanted => sub{
		      return unless -f && push @{$$table{-s $_}},
		      $File::Find::name;
	      }
	     }, $dir);
}

sub check_table{
	my $table=shift;
	
	foreach my $size(sort{$b<=>$a} keys %{$table}){
		next if @{$table->{$size}}<2;
		
		my %checksums;
		foreach my $file(@{$table->{$size}}){
			print STDERR "$size: $file\n";
			unless(open FILE,$file){
				warn "cant open $file: $!";
				next;
			}
			push @{$checksums{$md5->addfile(*FILE)->hexdigest}},$file;
			close FILE or die "cant close $file: $!";
		}
		&duplicate($_) foreach(grep{@{$_}>1}values %checksums);
	}
}

sub duplicate{
	my $dup=shift;
	
	$bytes_duplicated+=-s $_ foreach(@$dup[1..@$dup-1]);
	
	if($OPTIONS{RENAME_DUPLICATE} || $OPTIONS{DELETE_DUPLICATE}){
		foreach my $file(@$dup[1..@$dup-1]){
			if($OPTIONS{RENAME_DUPLICATE}){
				print "renaming: $file\n";
				unless(rename $file,"$file.DUP"){
					warn "cant rename $file to $file.DUP: $!";
				}
			}elsif($OPTIONS{DELETE_DUPLICATE}){
				print "removing: $file\n";
				unlink $file or warn "cant remove $file: $!";
			}
		}
	}else{
		local $,=" ";
		print @$dup,"\n";
	}
}

