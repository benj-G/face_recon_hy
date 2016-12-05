<?php

    //include "db_utils.php";

    function display_account_page() {
        // If client session is valid then display user page, else redirect to login page
        if(is_session_valid()) {
            echo(file_to_html('../account.html'));
        } else {
            header("Location: ../login.php");
            exit();
        }
    }

    function file_to_html($pagePath) {
        // Variables for substitution into HTML string
        $fname = $_SESSION['first_name'];
        $lname = $_SESSION['last_name'];
        $subs = array('$INTRO_TEXT' => "Welcome, $fname $lname! Any videos tied to your account will be posted in the Videos section below.",
                      '$HEADER_TEXT' => "User Page",
                      '$USER_VIDEOS' => build_videos());
        
        $htmlStr = "";
        
        try {
            $htmlStr = file_get_contents($pagePath);
        } catch(Exception $e) {
            trigger_error("Fatal error while attempting to load html from file.", E_USER_ERROR);
        }
        
        // Return HTML from file with variable substitutions defined above
        return strtr($htmlStr, $subs);
    }


    // Retrieves video file path and builds videos HTML element
    function build_videos() {                    
        $videoTag = "";

        // Defaulting to user_id 0 since it won't exist
        $userId = 0;
        
        $videoTag = "<ul class='list-inline'>";
        $vidSubTag = "<video width='320' height='240' controls /><source src='";

        // Retrieving map of video paths belonging to current user
        // If any videos exist for current user, then $vidMetaData
        // contains keys 'video_id' and 'out_video_file'
        $userId = get_current_user_id();
        $vidMetaData = get_vid_paths($userId);
        
        // If user has processed videos
        if($vidMetaData != false) {
            // Building HTML list element for each found video
            while($row = pgFetchAssoc($vidMetaData)) {
                $localpath = substr($row['out_video_file'], strpos($row['out_video_file'], "/data"), strlen($row['out_video_file']));
                $videoTag .= "<li>" . $vidSubTag . "../.." . $localpath . "'/></li>";
            }
        }
        
        return $videoTag .= "</ul>";
    }

?>
