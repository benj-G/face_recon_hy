<?php

    // Create new client session
    function create_client_session($email) {
        session_start();

        // Creating and storing new session ID
        $_SESSION['sId'] = session_id();
        
        // Storing user specific info
        $userInfo = get_user_info($email);
        $_SESSION['first_name'] = $userInfo['first_name'];
        $_SESSION['last_name'] = $userInfo['last_name'];
        $_SESSION['user_name'] = $email;
        
        // Note: Below is a float with fractional part removed
        //       Might be able to convert to int if no loss, but
        //       that won't work once the number of seconds gets
        //       larger than an int
        
        // Storing last access time in milliseconds for checking timeout
        $_SESSION['last_active'] = round(microtime(true)*1000);
    }

    // Verify session is active
    function is_session_active() {
        return (isset($_SESSION['sId']) && session_status() === PHP_SESSION_ACTIVE) ? true : false;
    }

    // Verify session id
    function verify_session_id() {
        
    }

    // Verify inactive session timeout limit not reached
    function check_session_timeout() {
    //    $timedout = true;
    //    
    //    if(session is timedout) {
    //        $timedout = true;
    //    } else {
    //        
    //    }
    //    
    //    return $timedout;
    }

    // Reset last session access time
    function poke_session() {
        // Store current access time
    }

    // End client session if one exists
    function end_session() {
        if(isset($_SESSION)) {
            session_destroy();
        }
    }

?>