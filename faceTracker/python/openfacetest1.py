#adapted from openface compare.py example
#for CS160 - team hellyeah 

import cv2
import os
import openface

print("****Openface test 1****")

# set directory paths
fileDir = os.path.dirname(os.path.realpath("/home/peter/src/openface/openface/openface"))
modelDir = os.path.join(fileDir, '..', 'models')
dlibModelDir = os.path.join(modelDir, 'dlib')
dlibFacePredictor = os.path.join(dlibModelDir, "shape_predictor_68_face_landmarks.dat")

imgPath = "/mnt/c/Users/pcurr/Dropbox/hellyeah/diana1.jpg"

align = openface.AlignDlib(dlibFacePredictor)

#load image
bgrImg = cv2.imread(imgPath)
if bgrImg is None:
    raise Exception("Unable to load image")

#convert to rgb
rgbImg = cv2.cvtColor(bgrImg, cv2.COLOR_BGR2RGB)

#get bounding rect
bb = align.getLargestFaceBoundingBox(rgbImg)
if bb is None:
    raise Exception("Unable to find a face")

#draw bounding rect
cv2.rectangle(bgrImg,(bb.left(),bb.top()), (bb.right(), bb.bottom()), (255,0,0), 2)

#get landmarks
landmarks = align.findLandmarks(rgbImg,bb)
if landmarks is None:
    raise Exception("Unable to get landmarks")

print("\nlandmarks:")
print(landmarks)

#draw landmarks
for landmark in landmarks:
    cv2.circle(bgrImg,landmark,2,(0,255,0),-1)

#save marked image
cv2.imwrite("/mnt/c/Users/pcurr/Dropbox/hellyeah/diana1_marked.jpg", bgrImg)


#get "rep" using neural net
openfaceModelDir = os.path.join(modelDir, 'openface')
networkModel = os.path.join(openfaceModelDir, 'nn4.small2.v1.t7')

imgDim = 96

alignedFace = align.align(imgDim, rgbImg, bb,
                          landmarkIndices=openface.AlignDlib.OUTER_EYES_AND_NOSE)
if alignedFace is None:
    raise Exception("Unable to align image: {}".format(imgPath))

net = openface.TorchNeuralNet(networkModel, imgDim)
rep = net.forward(alignedFace)

print("\nrep:")
print(rep)