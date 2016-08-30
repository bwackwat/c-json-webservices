#!/bin/bash

gcc -std=c99 -Wall -Wextra jsontest.c json.c -o jsontest

./jsontest
