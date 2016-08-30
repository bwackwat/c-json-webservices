#!/bin/bash

rm dvwebserver
rm dvclient
rm log.txt
killall dvwebserver

gcc -m64 -Wshadow -Wextra -Wpointer-arith -Wcast-qual -Wstrict-prototypes -g -pedantic -Wall dvwebserver.c util.c json.c -o dvwebserver
gcc -g -pedantic -Wall dvclient.c -o dvclient

./dvwebserver 8000 /home/vhasfchayesj7/workspace/c/public
./dvclient 127.0.0.1 8000

tail -f log.txt
