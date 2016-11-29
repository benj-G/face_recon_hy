<?php
    include "session_utils.php";
    include "db_utils.php";
    include "post_utils.php";

    $dir = "../data/vid/";
    $filename = $dir . basename($_FILES["file"]["name"]);
    if(isset($_COOKIE['c_sId'])){
     $cookie_sId = clean_post($_COOKIE['c_sId']);
     if(!check_existing_session($cookie_sId)) die($cookie_sId);
    }

    $user_name = get_username_from_sessions($cookie_sId);
    // If submit value is set, validate file type and extract file info
    // Uncomment this
    //if(isset($_POST["submit"])) {
    //
    //  add check for bad filename
        $query = "INSERT INTO VIDEO_METADATA(USER_ID, SRC_VIDEO_FILE, OUT_VIDEO_FILE, NUM_FRAMES, FRAME_RATE, FRAME_WIDTH, FRAME_HEIGHT) VALUES (1, '$filename', -1, -1, -1, -1, -1)";
        $dbConn = pgConnect();
        pgQuery($dbConn, $query);
        pgDisconnect($dbConn);
        make_cgi_post($cookie_sId, 3);
        die("done");
        // Check file type with ffprobe
        // If file is valid, store file info
    //}

    // Reject the upload if file size is > 10MB
    if($_FILES["file"]["size"] > 100000000) {
        die("Please upload a smaller file (< 10MB).");
    }

    if (!is_writeable($dir)) {
        mkdir("../data");
        mkdir("../data/vid");
       if (!is_writeable($dir . $_FILES['file']['name'])) die("Cannot write to destination file");
    }
    //rename("../data/vid/test.txt", "../data/vid/tmp.txt");
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

    function make_cgi_post($session_id, $video_id){
        $dest = "http://localhost/faceTracker/conduit.cgi";
        $data = array(
            'SESSIONID' => $session_id,
            'VIDEOID' => $video_id
            );
        //http://stackoverflow.com/questions/5647461/how-do-i-send-a-post-request-with-php
        $curl = curl_init($dest);
        curl_setopt($curl, CURLOPT_POST, true);
        curl_setopt($curl, CURLOPT_POSTFIELDS, http_build_query($data));    
        //curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
        $result = curl_exec($curl);
        if(!$result) die("failed request");
        curl_close($curl);
        return;
    }

?>