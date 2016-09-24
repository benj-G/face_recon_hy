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
