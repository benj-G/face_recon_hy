#!/bin/bash

cd ~/src/OpenFace/build/bin

./FaceLandmarkImg \
	--inputDir ~/input-images \
	--videoId 2 \
	--dbHost localhost \
	--dbName pipedream \
	--dbUser piper \
	--dbPassword letm3in
