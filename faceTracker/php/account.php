<?php

    //include "db_utils.php";

    function display_account_page() {
        // If client session is valid then display user page, else redirect to login page
        if(is_session_valid()) {
            echo(file_to_html('../account.html'));
        } else {
            header("Location: ../login.html");
            exit();
        }
    }

    function file_to_html($pagePath) {
        // Note: This will eventually inject HTML using a vector of videos 
        //       which will be built using whatever template we have
        // Video tag test template, will eventually just call build_videos()
        $testVid = "<video width='320' height='240' controls><source src='../data/vid/testVid.mp4' /></video>";
        
        // Variables for substitution into HTML string
        $fname = $_SESSION['first_name'];
        $lname = $_SESSION['last_name'];
        $subs = array('$INTRO_TEXT' => "Welcome, $fname $lname! Your videos will be posted somewhere here and other stuff.",
                      '$HEADER_TEXT' => "User Page",
                      '$USER_VIDEOS' => $testVid);
        
        $htmlStr = "";
        
        try {
            $htmlStr = file_get_contents($pagePath);
        } catch(Exception $e) {
            trigger_error("Fatal error while attempting to load html from file.", E_USER_ERROR);
        }
        
        // Return html from file with variable substitutions defined above
        return strtr($htmlStr, $subs);
    }

    function build_videos() {
        // Note: This function will build our video tag using a dynamic list of vids and a template
    }

?>