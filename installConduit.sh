#!/usr/bin/bash

sudo -v
gcc conduit.c -o conduit.cgi -I/usr/include/postgresql/ -L/usr/local/lib/postgresql/ -lpq
sudo cp -p conduit.cgi /usr/lib/cgi-bin
