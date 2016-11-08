<?php

    include "session_utils.php";

    ///////////
    // Script
    ////////

    // Ending client session
    end_session();
    //courtest of http://stackoverflow.com/questions/686155/remove-a-cookie
    if (isset($_COOKIE['c_sId'])) {
        unset($_COOKIE['c_sId']);
       setcookie('c_sId', '', time() - 3600, '/'); // empty value and old timestamp
       // die("in loop");
    }
    if (isset($_COOKIE['PHPSESSID'])) {
        unset($_COOKIE['PHPSESSID']);
       setcookie('PHPSESSID', '', time() - 3600, '/'); // empty value and old timestamp
       // die("in loop");
    }
    //else die("what?");
    // Redirect them to the landing page
    header("Location: ../index.html");
    
    // Exit script
    exit();
?>