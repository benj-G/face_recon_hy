///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016, Carnegie Mellon University and University of Cambridge,
// all rights reserved.
//
// THIS SOFTWARE IS PROVIDED �AS IS� FOR ACADEMIC USE ONLY AND ANY EXPRESS
// OR IMPLIED WARRANTIES WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY.
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Notwithstanding the license granted herein, Licensee acknowledges that certain components
// of the Software may be covered by so-called �open source� software licenses (�Open Source
// Components�), which means any software licenses approved as open source licenses by the
// Open Source Initiative or any substantially similar licenses, including without limitation any
// license that, as a condition of distribution of the software licensed under such license,
// requires that the distributor make the software available in source code format. Licensor shall
// provide a list of Open Source Components for a particular version of the Software upon
// Licensee�s request. Licensee will comply with the applicable terms of such licenses and to
// the extent required by the licenses covering Open Source Components, the terms of such
// licenses will apply in lieu of the terms of this Agreement. To the extent the terms of the
// licenses applicable to Open Source Components prohibit any of the restrictions in this
// License Agreement with respect to such Open Source Component, such restrictions will not
// apply to such Open Source Component. To the extent the terms of the licenses applicable to
// Open Source Components require Licensor to make an offer to provide source code or
// related information in connection with the Software, such offer is hereby made. Any request
// for source code or related information should be directed to cl-face-tracker-distribution@lists.cam.ac.uk
// Licensee acknowledges receipt of notices for the Open Source Components for the initial
// delivery of the Software.

//     * Any publications arising from the use of this software, including but
//       not limited to academic journal and conference publications, technical
//       reports and manuals, must cite at least one of the following works:
//
//       OpenFace: an open source facial behavior analysis toolkit
//       Tadas Baltru�aitis, Peter Robinson, and Louis-Philippe Morency
//       in IEEE Winter Conference on Applications of Computer Vision, 2016  
//
//       Rendering of Eyes for Eye-Shape Registration and Gaze Estimation
//       Erroll Wood, Tadas Baltru�aitis, Xucong Zhang, Yusuke Sugano, Peter Robinson, and Andreas Bulling 
//       in IEEE International. Conference on Computer Vision (ICCV),  2015 
//
//       Cross-dataset learning and person-speci?c normalisation for automatic Action Unit detection
//       Tadas Baltru�aitis, Marwa Mahmoud, and Peter Robinson 
//       in Facial Expression Recognition and Analysis Challenge, 
//       IEEE International Conference on Automatic Face and Gesture Recognition, 2015 
//
//       Constrained Local Neural Fields for robust facial landmark detection in the wild.
//       Tadas Baltru�aitis, Peter Robinson, and Louis-Philippe Morency. 
//       in IEEE Int. Conference on Computer Vision Workshops, 300 Faces in-the-Wild Challenge, 2013.    
//
///////////////////////////////////////////////////////////////////////////////
// FaceLandmarkImg.cpp : Defines the entry point for the console application for detecting landmarks in images.

//Adapted for "Team Hellyeah", SJSU, CS 160, Fall 2016

#include "LandmarkCoreIncludes.h"

// System includes
#include <fstream>

// OpenCV includes
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

// Boost includes
#include <filesystem.hpp>
#include <filesystem/fstream.hpp>

#include <dlib/image_processing/frontal_face_detector.h>

#include <tbb/tbb.h>

#include <FaceAnalyser.h>
#include <GazeEstimation.h>

#include "/usr/include/postgresql/libpq-fe.h"
#include <sstream>
#include <iostream>

using namespace std;

//face detection modules
LandmarkDetector::CLNF clnf_model;
cv::CascadeClassifier classifier;
dlib::frontal_face_detector face_detector_hog;

//video information
boost::filesystem::path inputDir;
string videoId;
int numFrames;

//database information
string dbHost, dbName, dbUser, dbPassword;
string dbConnectString;
vector<string> statements; //elements of the transaction

void makeDbConnectString()
{
    stringstream ss;
    ss << "host = " << "'" << dbHost << "'";
    ss << " dbname = " << "'" << dbName << "'";
    ss << " user = " << "'" << dbUser << "'";
    ss << " password = " << "'" << dbPassword << "'";
    dbConnectString = ss.str();
}
void getNumFrames() //from video_metadata table in db
{
    cout << "Begin getNumFrames" << endl;
    PGresult *db_result;

    PGconn *db_connection = PQconnectdb(dbConnectString.c_str());
    if (PQstatus(db_connection) != CONNECTION_OK)
    {
        printf("<P>[1]System error. Please contact customer support.<BR>");
        PQfinish(db_connection);
        exit(EXIT_FAILURE);
    }
    cout << "connected to db!" << endl;

    stringstream ss;
    ss << "SELECT num_frames FROM video_metadata WHERE video_id = " << videoId << ";";

    cout << "getting numFrames with sql statement: " << ss.str() << endl;

    db_result = PQexec(db_connection, ss.str().c_str());
    if(PQresultStatus(db_result) != PGRES_TUPLES_OK)
    {
        cout << "result status: " << PQresStatus(PQresultStatus(db_result)) << endl;
        cout << PQresultErrorMessage(db_result);
        printf("<p>[2]System error. Please contact customer service.<DB>");
        exit(EXIT_FAILURE);
    }

    ss.str("");ss.clear();
    ss << PQgetvalue(db_result,0,0);
    ss >> numFrames;

    cout << "Got numFrames: " << numFrames << endl;

    PQclear(db_result);
    PQfinish(db_connection);
}

class LandmarkFinder
{
    int currentFrame;

public:
    boost::filesystem::path currentFile;
    LandmarkFinder()
    {
        currentFrame = 0;
    }

    bool nextFrame()
    {
        currentFrame++;
        if (currentFrame > numFrames)
            return false;

        //construct current file path:
        stringstream ss;
        ss << videoId << "." << currentFrame << ".png";
        boost::filesystem::path file = boost::filesystem::path(ss.str());
        currentFile = inputDir / file;

        cout << "Constructed current image path: " << currentFile.string() << endl;

        return true;
    }
    void processFrame(LandmarkDetector::CLNF &clnf_model, cv::Vec6d pose)
    {
        stringstream ss;
        ss << "INSERT INTO video_data (VIDEO_ID, FRAME_NUM, HEAD_ROLL, HEAD_PITCH, HEAD_YAW, LEFT_PUPIL_OF_LOC_X, LEFT_PUPIL_OF_LOC_Y, RIGHT_PUPIL_OF_LOC_X, RIGHT_PUPIL_OF_LOC_Y, LEFT_PUPIL_FT_LOC_X, LEFT_PUPIL_FT_LOC_Y, RIGHT_PUPIL_FT_LOC_X, RIGHT_PUPIL_FT_LOC_Y, OF_DATA_POINT_1_LOC_X, OF_DATA_POINT_1_LOC_Y, OF_DATA_POINT_2_LOC_X, OF_DATA_POINT_2_LOC_Y, OF_DATA_POINT_3_LOC_X, OF_DATA_POINT_3_LOC_Y, OF_DATA_POINT_4_LOC_X, OF_DATA_POINT_4_LOC_Y, OF_DATA_POINT_5_LOC_X, OF_DATA_POINT_5_LOC_Y, OF_DATA_POINT_6_LOC_X, OF_DATA_POINT_6_LOC_Y, OF_DATA_POINT_7_LOC_X, OF_DATA_POINT_7_LOC_Y, OF_DATA_POINT_8_LOC_X, OF_DATA_POINT_8_LOC_Y, OF_DATA_POINT_9_LOC_X, OF_DATA_POINT_9_LOC_Y, OF_DATA_POINT_10_LOC_X, OF_DATA_POINT_10_LOC_Y, OF_DATA_POINT_11_LOC_X, OF_DATA_POINT_11_LOC_Y, OF_DATA_POINT_12_LOC_X, OF_DATA_POINT_12_LOC_Y, OF_DATA_POINT_13_LOC_X, OF_DATA_POINT_13_LOC_Y, OF_DATA_POINT_14_LOC_X, OF_DATA_POINT_14_LOC_Y, OF_DATA_POINT_15_LOC_X, OF_DATA_POINT_15_LOC_Y, OF_DATA_POINT_16_LOC_X, OF_DATA_POINT_16_LOC_Y, OF_DATA_POINT_17_LOC_X, OF_DATA_POINT_17_LOC_Y, OF_DATA_POINT_18_LOC_X, OF_DATA_POINT_18_LOC_Y, OF_DATA_POINT_19_LOC_X, OF_DATA_POINT_19_LOC_Y, OF_DATA_POINT_20_LOC_X, OF_DATA_POINT_20_LOC_Y, OF_DATA_POINT_21_LOC_X, OF_DATA_POINT_21_LOC_Y, OF_DATA_POINT_22_LOC_X, OF_DATA_POINT_22_LOC_Y, OF_DATA_POINT_23_LOC_X, OF_DATA_POINT_23_LOC_Y, OF_DATA_POINT_24_LOC_X, OF_DATA_POINT_24_LOC_Y, OF_DATA_POINT_25_LOC_X, OF_DATA_POINT_25_LOC_Y, OF_DATA_POINT_26_LOC_X, OF_DATA_POINT_26_LOC_Y, OF_DATA_POINT_27_LOC_X, OF_DATA_POINT_27_LOC_Y, OF_DATA_POINT_28_LOC_X, OF_DATA_POINT_28_LOC_Y, OF_DATA_POINT_29_LOC_X, OF_DATA_POINT_29_LOC_Y, OF_DATA_POINT_30_LOC_X, OF_DATA_POINT_30_LOC_Y, OF_DATA_POINT_31_LOC_X, OF_DATA_POINT_31_LOC_Y, OF_DATA_POINT_32_LOC_X, OF_DATA_POINT_32_LOC_Y, OF_DATA_POINT_33_LOC_X, OF_DATA_POINT_33_LOC_Y, OF_DATA_POINT_34_LOC_X, OF_DATA_POINT_34_LOC_Y, OF_DATA_POINT_35_LOC_X, OF_DATA_POINT_35_LOC_Y, OF_DATA_POINT_36_LOC_X, OF_DATA_POINT_36_LOC_Y, OF_DATA_POINT_37_LOC_X, OF_DATA_POINT_37_LOC_Y, OF_DATA_POINT_38_LOC_X, OF_DATA_POINT_38_LOC_Y, OF_DATA_POINT_39_LOC_X, OF_DATA_POINT_39_LOC_Y, OF_DATA_POINT_40_LOC_X, OF_DATA_POINT_40_LOC_Y, OF_DATA_POINT_41_LOC_X, OF_DATA_POINT_41_LOC_Y, OF_DATA_POINT_42_LOC_X, OF_DATA_POINT_42_LOC_Y, OF_DATA_POINT_43_LOC_X, OF_DATA_POINT_43_LOC_Y, OF_DATA_POINT_44_LOC_X, OF_DATA_POINT_44_LOC_Y, OF_DATA_POINT_45_LOC_X, OF_DATA_POINT_45_LOC_Y, OF_DATA_POINT_46_LOC_X, OF_DATA_POINT_46_LOC_Y, OF_DATA_POINT_47_LOC_X, OF_DATA_POINT_47_LOC_Y, OF_DATA_POINT_48_LOC_X, OF_DATA_POINT_48_LOC_Y, OF_DATA_POINT_49_LOC_X, OF_DATA_POINT_49_LOC_Y, OF_DATA_POINT_50_LOC_X, OF_DATA_POINT_50_LOC_Y, OF_DATA_POINT_51_LOC_X, OF_DATA_POINT_51_LOC_Y, OF_DATA_POINT_52_LOC_X, OF_DATA_POINT_52_LOC_Y, OF_DATA_POINT_53_LOC_X, OF_DATA_POINT_53_LOC_Y, OF_DATA_POINT_54_LOC_X, OF_DATA_POINT_54_LOC_Y, OF_DATA_POINT_55_LOC_X, OF_DATA_POINT_55_LOC_Y, OF_DATA_POINT_56_LOC_X, OF_DATA_POINT_56_LOC_Y, OF_DATA_POINT_57_LOC_X, OF_DATA_POINT_57_LOC_Y, OF_DATA_POINT_58_LOC_X, OF_DATA_POINT_58_LOC_Y, OF_DATA_POINT_59_LOC_X, OF_DATA_POINT_59_LOC_Y, OF_DATA_POINT_60_LOC_X, OF_DATA_POINT_60_LOC_Y, OF_DATA_POINT_61_LOC_X, OF_DATA_POINT_61_LOC_Y, OF_DATA_POINT_62_LOC_X, OF_DATA_POINT_62_LOC_Y, OF_DATA_POINT_63_LOC_X, OF_DATA_POINT_63_LOC_Y, OF_DATA_POINT_64_LOC_X, OF_DATA_POINT_64_LOC_Y, OF_DATA_POINT_65_LOC_X, OF_DATA_POINT_65_LOC_Y, OF_DATA_POINT_66_LOC_X, OF_DATA_POINT_66_LOC_Y, OF_DATA_POINT_67_LOC_X, OF_DATA_POINT_67_LOC_Y, OF_DATA_POINT_68_LOC_X, OF_DATA_POINT_68_LOC_Y) "
        << "VALUES ("
        << videoId << ", "
        << currentFrame << ", "

        //pose:
        << pose[5] << ", " //roll?
        << pose[3] << ", " //pitch?
        << pose[4] << ", " //yaw?

        //pupils:
        << -1 << ", " << -1 << ", "<< -1 << ", "<< -1 << ", "<< -1 << ", "<< -1 << ", "<< -1 << ", "<< -1
        ;

        //landmarks:
        int n = clnf_model.patch_experts.visibilities[0][0].rows;
        for (int i = 0; i < n; ++i)
        {
            ss << ", " << clnf_model.detected_landmarks.at<double>(i);
            ss << ", " << clnf_model.detected_landmarks.at<double>(i + n);
        }

        ss << ");";

        cout << "Constructed INSERT statement:\n" << ss.str() << endl;

        statements.push_back(ss.str());
    }

    void doTransaction()
    {
        if (statements.empty())
        {
            cout << "No statements to execute." << endl;
            return;
        }
        cout << "[LandmarkFinder] Beginning database transaction." << endl;

        PGresult *db_result;

        PGconn *db_connection = PQconnectdb(dbConnectString.c_str());
        if (PQstatus(db_connection) != CONNECTION_OK)
        {
            printf("<P>[3]System error. Please contact customer support.<BR>");
            PQfinish(db_connection);
            exit(EXIT_FAILURE);
        }

        db_result = PQexec(db_connection, "BEGIN");
        if (PQresultStatus(db_result) != PGRES_COMMAND_OK)
        {
            cout << PQresStatus(PQresultStatus(db_result)) << endl;
            cout << PQresultErrorMessage(db_result);
            printf("<P>[4]System error. Please contact customer support.<BR>");
            PQfinish(db_connection);
            exit(EXIT_FAILURE);
        }
        PQclear(db_result);

        for (string statement : statements)
        {
            PGresult *db_result = PQexec(db_connection, statement.c_str());
            if(PQresultStatus(db_result) != PGRES_COMMAND_OK)
            {
                cout << PQresStatus(PQresultStatus(db_result)) << endl;
                cout << PQresultErrorMessage(db_result);
                printf("<p>[5]System error. Please contact customer service.<DB>");
                exit(EXIT_FAILURE);
            }
            PQclear(db_result);
        }

        db_result = PQexec(db_connection, "END");
        PQclear(db_result);
        PQfinish(db_connection);
        cout << "Transaction committed. Inserted " << statements.size() << " rows." << endl;
    }

};

vector<string> get_arguments(int argc, char **argv)
{
    vector<string> arguments;

    for(int i = 0; i < argc; ++i)
    {
        arguments.push_back(string(argv[i]));
    }
    return arguments;
}

void convert_to_grayscale(const cv::Mat& in, cv::Mat& out)
{
    if(in.channels() == 3)
    {
        // Make sure it's in a correct format
        if(in.depth() != CV_8U)
        {
            if(in.depth() == CV_16U)
            {
                cv::Mat tmp = in / 256;
                tmp.convertTo(tmp, CV_8U);
                cv::cvtColor(tmp, out, CV_BGR2GRAY);
            }
        }
        else
        {
            cv::cvtColor(in, out, CV_BGR2GRAY);
        }
    }
    else if(in.channels() == 4)
    {
        cv::cvtColor(in, out, CV_BGRA2GRAY);
    }
    else
    {
        if(in.depth() == CV_16U)
        {
            cv::Mat tmp = in / 256;
            out = tmp.clone();
        }
        else if(in.depth() != CV_8U)
        {
            in.convertTo(out, CV_8U);
        }
        else
        {
            out = in.clone();
        }
    }
}

int main (int argc, char **argv)
{
    ////////////////////////////////////////////////////////////////////////
    //PARSE PARAMS
    ////////////////////////////////////////////////////////////////////////
    vector<string> arguments = get_arguments(argc, argv);
    LandmarkDetector::get_img_params_hellyeah(inputDir,
                                              dbHost, dbName, dbUser, dbPassword,
                                              videoId,
                                              arguments);
    LandmarkDetector::FaceModelParameters det_parameters(arguments);

    cout << "parameters parsed..." << endl;
    cout << "inputDir: " << inputDir.string() << endl
        << "db info: " << dbHost << ", " << dbName << ", " << dbUser << ", " << dbPassword << endl
        << "videoId: " << videoId << endl;

    // No need to validate detections, as we're not doing tracking
    det_parameters.validate_detections = false;

    ////////////////////////////////////////////////////////////////////////
    // LOAD MODULES
    ////////////////////////////////////////////////////////////////////////
    cout << "Loading face detection modules..." << endl;
    clnf_model = LandmarkDetector::CLNF(det_parameters.model_location);
    classifier = cv::CascadeClassifier(det_parameters.face_detector_location);
    face_detector_hog = dlib::get_frontal_face_detector();
    cout << "Modules loaded" << endl;

    ////////////////////////////////////////////////////////////////////////
    // PROCESS FRAMES
    ////////////////////////////////////////////////////////////////////////
    makeDbConnectString();
    getNumFrames();
    LandmarkFinder landmarkFinder;
    while(landmarkFinder.nextFrame())
    {
        // Loading image
        string file = landmarkFinder.currentFile.string();
        cv::Mat read_image = cv::imread(file, -1);
        cout << "Loaded file: " << file << endl;

        if (read_image.empty())
        {
            cout << "Could not read the input image" << endl;
            return 1;
        }

        // Loading depth file if exists (optional)
        cv::Mat_<float> depth_image;

        // Making sure the image is in uchar grayscale
        cv::Mat_<uchar> grayscale_image;
        convert_to_grayscale(read_image, grayscale_image);

        //approximate face position? ...not very good...
        float fx = 0, fy = 0, cx = 0, cy = 0;
        cx = grayscale_image.cols / 2.0f;
        cy = grayscale_image.rows / 2.0f;
        fx = 500 * (grayscale_image.cols / 640.0);
        fy = 500 * (grayscale_image.rows / 480.0);
        fx = (fx + fy) / 2.0;
        fy = fx;

	    cout << "Detecting faces in image..." << endl;
        // Detect faces in an image
        vector<cv::Rect_<double> > face_detections;

        if(det_parameters.curr_face_detector == LandmarkDetector::FaceModelParameters::HOG_SVM_DETECTOR)
        {
            vector<double> confidences;
            LandmarkDetector::DetectFacesHOG(face_detections, grayscale_image, face_detector_hog, confidences);
        }
        else
        {
            LandmarkDetector::DetectFaces(face_detections, grayscale_image, classifier);
        }

        cout << "Found " << face_detections.size() << " faces in this image\n";
        cout << "Detecting landmarks..." << endl;

        for(size_t face=0; face < face_detections.size() && face < 1; ++face) //hack to only do one face
        {
            // if there are multiple detections go through them
            bool success = LandmarkDetector::DetectLandmarksInImage(grayscale_image, depth_image, face_detections[face], clnf_model, det_parameters);

            // Estimate head pose
            cv::Vec6d headPose = LandmarkDetector::GetCorrectedPoseWorld(clnf_model, fx, fy, cx, cy);

            if(success)
            {
                landmarkFinder.processFrame(clnf_model, headPose);
            }
        }
    }
    landmarkFinder.doTransaction();
    return 0;
}

