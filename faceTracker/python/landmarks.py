#!/usr/bin/env python

import os
import argparse
import cv2
import psycopg2 as pg

def rect_contains(rect, point) :
    if point[0] < rect[0] :
        return False
    elif point[1] < rect[1] :
        return False
    elif point[0] > rect[2] :
        return False
    elif point[1] > rect[3] :
        return False
    return True

# PARSE ARGUMENTS
print "Parsing arguments..."
parser = argparse.ArgumentParser()
parser.add_argument("--videoId", required=True)
parser.add_argument("--inputDir", required=True, help="Path to directory containing image files for frames of this video")
parser.add_argument("--dbName", required=True)
parser.add_argument("--dbUser", required=True)
parser.add_argument("--dbPassword", required=True)
args = vars(parser.parse_args())
videoId = args['videoId']
inputDir = args['inputDir']
dbName = args['dbName']
dbUser = args['dbUser']
dbPassword = args['dbPassword']

# CONNECT TO DB
print "Connecting to DB..."
try:
    conn = pg.connect("dbname={n} user={u} password={p}".format(n=dbName, u=dbUser, p=dbPassword))
except Exception as e:
    print "Error connecting to db", e
    exit(1)

cur = conn.cursor()

# GET METADATA
selectString = "SELECT num_frames,frame_width,frame_height FROM video_metadata WHERE video_id={}".format(videoId)
print "SELECT command for metadata: ", selectString
try:
    cur.execute("SELECT num_frames,frame_width,frame_height FROM video_metadata WHERE video_id={}".format(videoId))
except Exception as e:
    print "Error selecting from metadata table", e
    exit(1)

metadata = cur.fetchone()
if metadata is None: # make sure that worked
    print "metadata is none!"
    exit(1)
print "metadata: ", metadata

(numFrames, frameWidth, frameHeight) = metadata
print "{} frames available for this video".format(numFrames)
print "frame width:",frameWidth
print "frame height:",frameHeight

cur.execute("BEGIN")

# LOOP OVER FRAMES, GETTING LANDMARKS AND PUPILS, CREATING TRIANGLES, AND DRAWING MARKED IMAGES
for curFrame in range(1,numFrames+1):

    print "Processing frame ", curFrame

    # CREATE IMG PATH
    imgFileName = "{v}.{f}.png".format(v=videoId, f=curFrame)
    imgPath = os.path.join(inputDir, imgFileName)
    print imgPath

    # LOAD IMAGE
    bgrImg = cv2.imread(imgPath)
    if bgrImg is None:
        raise Exception("Unable to load image")

    # convert to rgb
    rgbImg = cv2.cvtColor(bgrImg, cv2.COLOR_BGR2RGB)

    # get bounding rect
    bb = align.getLargestFaceBoundingBox(rgbImg)
    if bb is None:
        raise Exception("Unable to find a face")

cur.execute("END")
cur.close()
conn.close()
print "script finished!"
