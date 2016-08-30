#!/bin/bash
set -e

if [ ! -f 'db-6.2.23.NC.tar.gz' ]; then
	wget http://download.oracle.com/berkeley-db/db-6.2.23.NC.tar.gz 
fi

if [ ! -d 'db-6.2.23.NC' ]; then
	tar -xf db-6.2.23.NC.tar.gz
fi
