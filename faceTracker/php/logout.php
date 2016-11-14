<?php

    include "session_utils.php";
    include "db_man.php";
    include "post_utils.php";
    ///////////
    // Script
    ////////
    //Whole script is very susceptiable if someone modifies the cookies. We may have duplicates. 1 possible way to to make 2 candidate keys that would return errors on duplicate user sessions
    // Ending client session
    end_session();
    $name = clean_post($_COOKIE['c_sId']);
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
    $query = "DELETE FROM SESSIONS WHERE SESSION_ID = (SELECT SESSION_ID FROM SESSION_LOOKUP WHERE SESSION_ID = '$name')";
    $dbConn = pgConnect();
    pgQuery($dbConn, $query);
    pgDisconnect($dbConn);
    // Redirect them to the landing page
    header("Location: ../index.html");
    
    // Exit script
    exit();
?>