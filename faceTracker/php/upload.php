<?php

    $dir = "../data/vid/";
    $filename = $dir . basename($_FILES["file"]["name"]);

    // If submit value is set, validate file type and extract file info
    if(isset($_POST["submit"])) {
        // Check file type with ffprobe
        // If file is valid, store file info
    }

    // Reject the upload if file size is > 10MB
    if($_FILES["file"]["size"] > 100000000) {
        echo("Please upload a smaller file (< 10MB).");
    }

    // Check if name already in use
    check_filename_duplicity($filename); 

    // Attempt to upload file and echo success or failure
    if(move_uploaded_file($_FILES["file"]["tmp_name"], $filename)) {
        echo("Your file was uploaded successfully!");
    } else {
        echo("There was an error uploading your file. Please verify the file type and try again.");
    }

    // If file name already in use, append next unused integer
    function check_filename_duplicity($filename) {
        $i = 1;
        
        while(file_exists($filename)) {
            // Note: appending some user specific id may work better
            //       or may not be a problem once we have our
            //       user video dir structure implemented
            $filename .= strval($i);
            $i++;
        }
    }

?>