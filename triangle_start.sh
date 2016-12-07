#!/bin/bash

$1;

./triangles.py \
	--videoId $1 \
	--inputDir /var/www/html/data/$1/ \
	--dbName pipedream \
	--dbUser piper \
	--dbPassword letm3in
