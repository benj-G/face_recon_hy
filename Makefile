CC=gcc
LDIR=/usr/lib/postgresql/
IDIR=/usr/include/postgresql/
create_conduit:
	gcc -I $(IDIR) conduit.c -o conduit.cgi -L$(LDIR) -lpq
clean: 
	rm conduit.cgi