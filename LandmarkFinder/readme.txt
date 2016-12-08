OpenFace code modified for Team Hell Yeah.

The FaceLandmarkImg program has been modified to put the landmarks in a postgres database.

openface_setup.sh contains commands to install OpenFace. You can try running the script, or just step through the commands manually.

After installing OpenFace, overwrite the original code with the provided code in OpenFace/, then rebuild FaceLandmarkDetector with:
cd OpenFace/build
cmake ..
make FaceLandmarkImg

The executable will appear in OpenFace/build/bin

hellyeah_test.sh contains an example of how to call the modified FaceLandmarkImg program.

The main code changes are in OpenFace/exe/FaceLandmarkImg/FaceLandmarkImg.cpp