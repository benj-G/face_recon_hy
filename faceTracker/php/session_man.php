<?php

   // include_once "db_man.php";

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
        $_SESSION['ipaddr'] = filter_var($_SERVER["REMOTE_ADDR"], FILTER_VALIDATE_IP);

        // insert information into sessions and session_lookup table
        
        $dbConn = pgConnect();

        $query1 = "SELECT * FROM SESSIONS WHERE SESSION_ID = '".$_SESSION['sId']."'";

        $result = pgQuery($dbConn, $query1);

        if(pg_num_rows($result) == 0){
        $query2 = "INSERT INTO session_lookup(session_id, user_name) VALUES ('".$_SESSION['sId']."', '".$_SESSION['user_name']."')"; // sanitize?
        $query1 = "INSERT INTO sessions(session_id, session_time, ipaddr) VALUES ('".$_SESSION['sId']."', now(), '".$_SESSION['ipaddr']."')";

        try{
            pgQuery($dbConn, $query1); //error
            pgQuery($dbConn, $query2); //error
        }catch(Exception $e){

        }
        }
        pgDisconnect($dbConn);

        $cookie_sId = $_SESSION['sId'];
        setcookie('c_sId', $cookie_sId, 0, '/');
    }

    function reload_client_session($email){
        session_start();

        // Creating and storing new session ID
        $_SESSION['sId'] = session_id();
        // Storing user specific info
        $userInfo = get_user_info($email);
        $name = $userInfo['user_name'];
        //$time = 
        $current_time = time();

        // Delete session if session has expired. Over 30 minutes.
        //if(($current_time - $time) > 1){ // change to 1800
            //end_session();
            $query = "DELETE FROM SESSIONS WHERE SESSION_ID = (SELECT SESSION_ID FROM SESSION_LOOKUP WHERE USER_NAME = '$name') AND now() > (SESSION_TIME + '30 MINUTES')"; // deal when session lookup table has more than 1 use name
            $dbConn = pgConnect();
            $results = pg_query($dbConn, $query);
            pgDisconnect($dbConn);
            //die(var_dump(pg_fetch_all($results)));
        //}
        $_SESSION['first_name'] = $userInfo['first_name'];
        $_SESSION['last_name'] = $userInfo['last_name'];
        $_SESSION['user_name'] = $email;
        
        // Note: Below is a float with fractional part removed
        //       Might be able to convert to int if no loss, but
        //       that won't work once the number of seconds gets
        //       larger than an int
        
        // Storing last access time in milliseconds for checking timeout
        $_SESSION['last_active'] = round(microtime(true)*1000);
        $_SESSION['ipaddr'] = filter_var($_SERVER["REMOTE_ADDR"], FILTER_VALIDATE_IP);
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