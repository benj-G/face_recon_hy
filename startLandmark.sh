#!/bin/bash

$1;

./FaceLandmarkImg \
	--inputDir /var/www/html/data/$1/ \
	--videoId $1 \
	--dbHost localhost \
	--dbName pipedream \
	--dbUser piper \
	--dbPassword letm3in
