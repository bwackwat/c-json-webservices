#!/bin/bash

gcc -std=c99 -Wall -Wextra -lodbc sqltest.c -o sqltest

./sqltest
