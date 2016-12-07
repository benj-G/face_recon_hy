# face_recon_hy

# Brought to you by Team Hell Yeah!

### Loading the Test Website
The website is up! It's still a very rough and near featureless version of our project's web portal, but for now it will do.

##### Instructions:
1. Place the faceTracker folder into your '/var/www/html/' directory.
2. Open a terminal and enter `sudo touch /etc/apache2/sites-available/faceTracker.conf`
3. Open the file in an editor with `sudo gedit /etc/apache2/sites-available/faceTracker.conf`
4. Add this text to the file and save: 
```
<VirtualHost *:80>
    ServerName faceTracker
    DocumentRoot /var/www/html/faceTracker
    ErrorLog ${APACHE_LOG_DIR}/error.log
    CustomLog ${APACHE_LOG_DIR}/access.log combined
</VirtualHost>
```
5. Now let Apache know that the site exists: `sudo a2ensite /etc/apache2/sites-available/faceTracker.conf`
6. Restart the Apache service: `sudo service apache2 restart`
7. Now add the project to your hosts file: go to `sudo gedit /etc/hosts` and add the line `127.0.0.1 faceTracker`
8. Go to http://faceTracker or localhost/faceTracker and you should see the landing page!

### Setting up openssl
1. Go to https://app.box.com/s/4t01n742w1afxne2ywk2gmo1t3etgarl
2. Pick the right folder for your operating system
3. Copy the bin folder to you where your other folders in var/www/html/faceTracker (This is temporary I believe there is a designated place on servers for executables)

### Building Conduit
1. Navigate to directory containing conduit.c
2. run make
	- the makefile uses the default include and library directories, it needs access to libpq and the header file libpq-fe

### Preparing Openface
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

### Setting up cgi-bin
The following executables/folders should be placed in your servers cgi-bin and marked as executable.
-conduit.cgi
-FaceLandmarkImg - build with Peter's code
-startLandmark.sh
-triangles.py
-triangle_start.sh
-mode
	contains the xml files from openface

### Importing database
shadestream_schema.sql must be imported into a database called pipedream with user piper and password letm3in. This can be changed in dbConn.php. It is hardcoded in most scripts and must be changed there as well.
