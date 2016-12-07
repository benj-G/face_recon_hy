/**********************************************************************
*
* CS 160 Section 0x Fall 2016
* Programming Project 3: Extract Still Images from Video
* Date Due: Wednesday, December 7, 2016
* Submitted to Prof. Robert Bruce
* Submitted by Team Hell Yeah: Tom, Peter, Ben, Manisha, David
* 
* conduit.c
* This program is the CGI binary that is launched by the Apache2 HTTP 
* server. 
*
* Pre-conditions:
* Prior to calling this program, the user video should be uploaded to
* to the Apache2 HTTP server and stored and stored in the server's file
* system. The VIDEO_METADATA table in the database should have a row 
* for the uploaded video that has a unique VIDEO_ID for the video, a
* USER_ID identifying the user who uploaded the video, and the 
* SRC_VIDEO_FILE providing the fully pathed filename for the video, 
* where the video file is located in the server file system.
*
* Function:
* conduit will process the video file, first determining if it is a 
* valid video file. If it is, it will further process the file to 
* obtain video metadata including fps, width and height. Then it will
* extract each frame as a image for locating facial and pupil 
*  reference points. It will then draw the facial reference points and 
* connect them as Delaney triangles on each frame image. Finally, the 
* augmented frame images are assembled together as a video that can 
* be played by the user.
*
* Input (provided via standard input from calling script): 
* ampersand separated key-value pairs for
*  - the VIDEO_ID of the uploaded video
*  - the SESSION_ID of the user's active session
* For example: VIDEOID=234&SESSIONID=5432
*
* Output (provided via standard output):
*  - explanatory status message as HTML page
*  - TBD
*
* Post-conditions:
* As per the specification for CGI applications, this program will
* provide via standard output the HTML page that can be displayed in 
* the user's browser indicating the success (or error condition) of
* its processing.
*
* Version History:
* 0.1.0 - David Burke - Initial Release
* This version is focused on being able to validate the video file 
* and create the video metadata. No session validation.
*
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include "libpq-fe.h"
#include <errno.h>

#define TRUE 1
#define FALSE 0
#define MAX_COMMAND_LINE 255
#define MAX_DB_STATEMENT_BUFFER_LENGTH 5000
#define MAXLEN 127 
#define MAXPARAMLENGTH 5
#define MAX_VIDEO_ID_LENGTH 20
#define MAX_SESSION_ID_LENGTH 255
#define MAX_VIDEO_FILENAME_LENGTH 255
#define MAXINPUT MAXLEN+2
/* 1 for added line break, 1 for trailing NUL */
// #define DATAFILE "../data/data.txt"

//forward declarations
int processVideo(long);
int validSessionID(char *);
void back_to_login();
void unencode(char *src, char *last, char *dest)
{
 for(; src != last; src++, dest++)
   if(*src == '+')
     *dest = ' ';
   else if(*src == '%') {
     int code;
     if(sscanf(src+1, "%2x", &code) != 1) code = '?';
     *dest = code;
     src +=2; }     
   else
     *dest = *src;
 *dest = '\n';
 *++dest = '\0';
}

int main(void)
{
  char showDetails = 0;
  char *lenstr;
  char input[MAXINPUT], data[MAXINPUT];
  long len;
  char keyName[9] = "";
  char keyValue[9] = "";
  long videoID = -1;
  char sessionID[255] = "";
  const char *VIDEOID = "VIDEOID=";
  const char *SESSIONID = "SESSIONID=";
  const char *DETAILS = "DETAILS=";

  printf("%s%c%c\n", "Content-Type:text/html;charset=iso-8859-1",13,10);
  printf("<TITLE>Response</TITLE>\n");
  lenstr = getenv("CONTENT_LENGTH"); // CONTENT_LENGTH env var has length of input
  if(lenstr == NULL || sscanf(lenstr,"%ld",&len)!=1 || len > MAXLEN)
    printf("<P>System Error - incorrect invocation - wrong FORM probably.");
  else 
  {
    fgets(input, len+1, stdin);
 //   printf("<p> the std input is %s.<BR>", input);
    unencode(input, input+len, data);
    // extract values for VIDEO_ID and SESSION_ID from input string
    char *token;
    char keyName[9] = "";
    const char kvDelim[2] = "&";
    const char kvAssignOp[2] = "=";
    token = strtok(input, kvDelim);
    if(token == NULL)
      printf("<p>System Error - Incorrect invocation - wrong FORM probably.");
    else
    {
      while(token != NULL)
      {
        strncpy(keyName, token, strlen(VIDEOID));
        keyName[strlen(VIDEOID)] = '\0';
        if(strcmp(keyName, VIDEOID) == 0) 
        {
          sprintf(keyValue, "%.*s", (int)(strlen(token) - strlen(VIDEOID)), token + strlen(VIDEOID));
          videoID = strtol(keyValue, NULL, 10);
        }
        else
        {
          strncpy(keyName, token, strlen(SESSIONID));
          keyName[strlen(SESSIONID)] = '\0';
          if(strcmp(keyName, SESSIONID) == 0) 
          {
            sprintf(sessionID, "%.*s", (int)(strlen(token) - strlen(SESSIONID)), token + strlen(SESSIONID));
         //   sessionID = strtol(keyValue, NULL, 10);
          }
          else
          {
            strncpy(keyName, token, strlen(DETAILS));
            keyName[strlen(DETAILS)] = '\0';
            if(strcmp(keyName, DETAILS) == 0) 
            {
              sprintf(keyValue, "%.*s", (int)(strlen(token) - strlen(DETAILS)), token + strlen(DETAILS));
              if(strlen(keyValue) > 0)
                showDetails = 1;
            } 
          }
        }
        token = strtok(NULL, kvDelim);
      }
      if(showDetails) 
      {
        printf("<p>Video ID: %d<br>", (int)videoID);
        printf("<p>Session ID: %s<br>", sessionID);
      }
      // validate session id
      if(validSessionID(sessionID) != 0)
      {
        printf("<p>Unexpected values. Please contact Customer Support<BR>");
        back_to_login();
        exit(EXIT_FAILURE);
      }
      // process video by video ID
      if(processVideo(videoID) != 0) printf("<p>Could not process video<BR>");
    }
  }
  return 0;
}

int validSessionID(char * in_sessionID)
{
  char * req_ipaddr;
  char * sessionValid;
  PGconn *db_connection;
  PGresult *db_result;
  char db_statement[MAX_DB_STATEMENT_BUFFER_LENGTH],
       session_ID[MAX_SESSION_ID_LENGTH];
  char sess_timestamp[MAXPARAMLENGTH],
       sess_ipaddr[MAXPARAMLENGTH];

  const char *parameter_values[2];
//  int parameter_lengths[2];
//  int parameter_formats[2];

  req_ipaddr = getenv("REMOTE_ADDR");
  printf("<p> the remote addr is '%s'.<BR>", req_ipaddr);
  db_connection = PQconnectdb("host = 'localhost' dbname = 'pipedream' user = 'piper' password = 'letm3in'");
  if(PQstatus(db_connection) != CONNECTION_OK)
  {
    printf("<p>System error connection failure. Please contact Customer Support.<BR>");
    PQfinish(db_connection);
    back_to_login();
    exit(EXIT_FAILURE);
  }

  // see if the session is valid by looking for it in the SESSIONS table and checking that the session create timestamp 
  // is less than 30 minutes old and that the IP address of the caller matches

  snprintf(session_ID, MAX_SESSION_ID_LENGTH, "%s", in_sessionID);
  parameter_values[0] = &session_ID[0];
  parameter_values[1] = &req_ipaddr[0];
  strncpy(&db_statement[0], "SELECT extract(minute from (current_timestamp - session_time)) <= 30 from sessions where session_id = $1 and ipaddr = $2", MAX_DB_STATEMENT_BUFFER_LENGTH);
  db_result = PQexecParams(db_connection, db_statement, 2, NULL, parameter_values, NULL, NULL, 0);
  if(PQresultStatus(db_result) != PGRES_TUPLES_OK || PQntuples(db_result) != 1)
  {
    printf("<p>System error. Please contact Customer Service.<BR>");
    back_to_login();
    exit(EXIT_FAILURE);
  }
  else
  {
    sessionValid = PQgetvalue(db_result, 0, 0);
    if(strncmp(sessionValid, "t", 1) != 0)
    {
      printf("<p>System error validation failure. Please contact Customer Service.<br>");
      back_to_login();
      exit(EXIT_FAILURE);
    } 
  }

  PQfinish(db_connection);

  return 0;
}

int processVideo(long in_videoID)
{
  PGconn *db_connection;
  PGresult *db_result;
  char db_statement[MAX_DB_STATEMENT_BUFFER_LENGTH], 
//       command_buffer[MAX_COMMAND_BUFFER_LENGTH], 
       video_ID[MAX_VIDEO_ID_LENGTH], 
       video_filename[MAX_VIDEO_FILENAME_LENGTH],
       path_name[MAX_COMMAND_LINE + MAX_VIDEO_ID_LENGTH];
  char fr_Height[MAXPARAMLENGTH],
       fr_Width[MAXPARAMLENGTH],
       fr_perSec[MAXPARAMLENGTH],
       nbr_fr[MAXPARAMLENGTH];
  const char *parameter_values[6];
  int parameter_lengths[2];
  int parameter_formats[2];
  int menu, command, row, num_rows;

  db_connection = PQconnectdb("host = 'localhost' dbname = 'pipedream' user = 'piper' password = 'letm3in'");
  if(PQstatus(db_connection) != CONNECTION_OK)
  {

    printf("<P>System error. Please contact customer support.<BR>");
    PQfinish(db_connection);
    back_to_login();
    exit(EXIT_FAILURE);
  }
  // see if the videoID is valid by getting the src_video_file 
  snprintf(video_ID, MAX_VIDEO_ID_LENGTH, "%ld", in_videoID);
  parameter_values[0] = &video_ID[0];
  strncpy(&db_statement[0], "SELECT src_video_file FROM video_metadata WHERE video_id = $1", MAX_DB_STATEMENT_BUFFER_LENGTH);
  db_result = PQexecParams(db_connection, db_statement, 1, NULL, parameter_values, NULL, NULL, 0);
  if(PQresultStatus(db_result) != PGRES_TUPLES_OK || PQntuples(db_result) != 1)
  {
    printf("<p>System error. Please contact customer service.<BR>");
  }
  else
  {
    // assuming a singular result
    sprintf(video_filename, "%s", PQgetvalue(db_result, 0, 0));

    FILE *fp;
    FILE *mov;
    int status;
    char entry[MAXLEN];
    int entryCount = 0;
    char *token;
    char ffCommand[MAX_COMMAND_LINE];
    int frameHeight = -1;
    int frameWidth = -1;
    float fps = -1.0;
    int numFrames = -1;
    char * ech; // error location
    
    snprintf(ffCommand, MAX_COMMAND_LINE, "%s %s", "ffprobe -v error -count_frames -select_streams v:0 -show_entries stream=height,width,avg_frame_rate,nb_read_frames -of default=noprint_wrappers=1:nokey=1", video_filename);

    fp = popen(ffCommand, "r");
    if(fp == NULL)
    {
      printf("<p>System error. Please contact customer support.<BR>");
      back_to_login();
      exit(EXIT_FAILURE);
    }
    while(fgets(entry, MAXLEN, fp) != NULL)
    {
      entryCount++;
      switch(entryCount)
      {
        case 1 :
          frameHeight = (int)strtol(entry, NULL, 10);
          break;
        case 2 :
          frameWidth = (int)strtol(entry, NULL, 10);
          break;
        case 3 :
          token = strtok(entry, "/");
          if(token == NULL)
          {
            printf("<p>System error. Please contact customer support.<BR>");
            break;
          }
          char* tmp;
          float numerator = strtof(token, &tmp);
          token = strtok(NULL, "/");
          float denominator = strtof(token, &tmp);
          fps = numerator / denominator;
          printf("Frames Per Seoncd: %f", fps);
          break;
        case 4 :
          numFrames = (int)strtol(entry, NULL, 10);
          break;
        default :
          printf("<p>System error. Please contact customer support.<BR>");
      }
    }
    if(entryCount == 0)
    {
      printf("<p>Uploaded video file could not be processed, video format not detected.<BR>");
      remove(video_filename);
      back_to_login();
      exit(EXIT_FAILURE);
    }
//    printf("<p> width is %d, height is %d, fps = %f, #frames is %d<BR>", frameWidth, frameHeight, fps, numFrames);
    char video_filename_cp[2180];
    char *base_dir;
    char* img_subdir = calloc(2180, sizeof(char)); // handle long paths
    printf("<p>video_filename before dirname() call is %s<BR>", video_filename);
    strncpy(video_filename_cp, video_filename, 1280);
    base_dir = dirname(video_filename_cp);
    printf("<p>video_filename after dirname() call is %s<BR>", video_filename);

    strncat(img_subdir, base_dir, 1024); 
    strncat(img_subdir, "/", 1);
    strncat(img_subdir, video_ID, MAX_VIDEO_ID_LENGTH);
    printf("<p>mage subdirectory is %s<BR>", img_subdir);

    // write metadata to the database
    sprintf(fr_Height, "%d", frameHeight);
    sprintf(fr_Width, "%d", frameWidth);
    sprintf(fr_perSec, "%f", fps);
    sprintf(nbr_fr, "%d", numFrames);
    sprintf(video_ID, "%ld", in_videoID);
    sprintf(path_name, "%s/%s.mp4", img_subdir, video_ID);

    strncpy(&db_statement[0], "UPDATE video_metadata SET num_frames = $1, frame_rate = $2, frame_width = $3, frame_height = $4, out_video_file=$6 WHERE video_id = $5", MAX_DB_STATEMENT_BUFFER_LENGTH);
    parameter_values[0] = &nbr_fr[0];
    parameter_values[1] = &fr_perSec[0];
    parameter_values[2] = &fr_Width[0];
    parameter_values[3] = &fr_Height[0];
    parameter_values[4] = &video_ID[0];
    parameter_values[5] = &path_name[0];

    db_result = PQexecParams(db_connection, db_statement, 6, NULL, parameter_values, NULL, NULL, 0);
    
    if(PQresultStatus(db_result) != PGRES_COMMAND_OK)
    {
      printf("<p>System error. Please contact customer service.<DB>");
      back_to_login();
      exit(EXIT_FAILURE);
    }
    status = pclose(fp);
    if(status == -1)
    {
      printf("<p>System error. Please contact customer support<BR>");
      back_to_login();
      exit(EXIT_FAILURE);
    }

    // create a subdirectory for the video's images from frames
    struct stat file_stat;
   
    int dir_exists = FALSE;
    
    
    if(stat(&img_subdir[0], &file_stat) == 0)
    {
      dir_exists = TRUE;
      printf("<p>image subdirectory already exists<BR>");
    }
    else
    {
      if(mkdir(img_subdir, 0755) == 0)
      {
        dir_exists = TRUE;
        printf("<p>mkdir() used to create image subdirectory<BR>");
      }
      else
      {
        printf("<p>mkdir() failed creating %s\n", img_subdir);
        printf("<p>System error. Please contact customer support. %s<BR>", strerror(errno));
        back_to_login();
        exit(EXIT_FAILURE);
      }
    }
    // extract the video frames to still images in the created subdirectory
    snprintf(ffCommand, MAX_COMMAND_LINE, "ffmpeg -v error -i %s -vf fps=%f %s/%s.%%d.png", video_filename, fps, img_subdir, video_ID);
    printf("<p>=%s=<BR>", ffCommand);

    fp = popen(ffCommand, "r");
    if(fp == NULL)
    {
      printf("<p>System error. Please contact customer support.<BR>");
      back_to_login();
      exit(EXIT_FAILURE);
    }
    status = pclose(fp);
    if(status == -1)
    {
      printf("<p>System error. Please contact customer support<BR>");
      back_to_login();
      exit(EXIT_FAILURE);
    }

    // Store Facial points in database
    char* script_info = calloc(250, sizeof(char));
    sprintf(script_info, "./startLandmark.sh %s -j4", video_ID);
    //sprintf(script_info, "./hellyeah_test.sh");
    fp = popen(script_info, "r");
    if(fp == NULL)
      {
        printf("<p>System error. Please contact customer support.<BR>");
        back_to_login();
        exit(EXIT_FAILURE);
      }
      status = pclose(fp);
      if(status == -1)
      {
        printf("<p>System error. Please contact customer support<BR>");
        back_to_login();
        exit(EXIT_FAILURE);
      }
    printf("\nString not printing:%s\n", script_info);

    char* call_draw = calloc(250, sizeof(char));
    sprintf(call_draw, "./triangle_start.sh %s > %s/%s", video_ID, img_subdir, "log.txt");
    printf("<br><br><br><br><br>Command: %s<br><br><br>", call_draw);
    fp = popen(call_draw, "r");
    if(fp == NULL)
      {
        printf("<p>System error. Please contact customer support.<BR>");
        back_to_login();
        exit(EXIT_FAILURE);
      }
      status = pclose(fp);
      if(status == -1)
      {
        printf("<p>System error. Please contact customer support<BR>");
        back_to_login();
        exit(EXIT_FAILURE);
      }

    //Stitch together video files
    snprintf(ffCommand, MAX_COMMAND_LINE, "ffmpeg -v error -framerate %f -i %s/%s.%%d.png -c:v libx264 %s/%s.mp4", fps, img_subdir, video_ID, img_subdir, video_ID);
    printf("\n\n%s\n\n", ffCommand);
    fp = popen(ffCommand, "r");
    if(fp == NULL)
      {
        printf("<p>System error. Please contact customer support.<BR>");
        back_to_login();
        exit(EXIT_FAILURE);
      }
      status = pclose(fp);
      if(status == -1)
      {
        printf("<p>System error. Please contact customer support<BR>");
        back_to_login();
        exit(EXIT_FAILURE);
      }
    // move old video file into new directory
    char* mov_dir = calloc(MAX_VIDEO_FILENAME_LENGTH, sizeof(char));
    sprintf(mov_dir, "%s/%s", img_subdir, "oldvfile");
    printf(" - %s - ", mov_dir);
    rename(video_filename, mov_dir);
    parameter_values[0] = mov_dir;
    parameter_values[1] = video_ID;

    strncpy(&db_statement[0], "UPDATE VIDEO_METADATA SET SRC_VIDEO_FILE=$1 WHERE VIDEO_ID=$2", MAX_DB_STATEMENT_BUFFER_LENGTH);
    db_result = PQexecParams(db_connection, db_statement, 2, NULL, parameter_values, NULL, NULL, 0);
    
    if(PQresultStatus(db_result) != PGRES_COMMAND_OK)
    {
      printf("<p>System error. Please contact customer service.<DB>");
      exit(EXIT_FAILURE);
    }
  }
  PQclear(db_result);
  PQfinish (db_connection);
  back_to_login();
  return 0;
}

void back_to_login(){
    printf("<meta http-equiv=\"refresh\" content=\"4; URL=login.php\">");
}