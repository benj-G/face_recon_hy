<?php

    include "session_man.php";

    // Create new client session if inactive
    function create_session($email) {
        if(!is_session_active()) {
            create_client_session($email); 
        }
    }

    function recreate_session($email){
        if(!is_session_active()) {
            reload_client_session($email); 
        }
    }

    // Verify session is active and valid
    function is_session_valid() {
        return (is_session_active()) ? true : false;
    }

    // Check session id in cookie. Check NOTADDED - ipaddress and session id
    function check_existing_session($cookie_containing_session){
        //TODO: santize to be safe
        
        $valid_session = false;

        $query = ("SELECT * FROM sessions WHERE session_id='$cookie_containing_session'");
        $dbConn = pgConnect();
        $results = pgQuery($dbConn, $query);
        //if (mysql_num_rows($result)==0) $valid_session = false; //not needed just for readability

        $tuple = pg_fetch_assoc($results);
        if(count($tuple) == 0) return $valid_session;
        if($tuple['session_id'] == $cookie_containing_session && $tuple['ipaddr']==$_SERVER['REMOTE_ADDR']) $valid_session = true; // if false delete session?
        // Could potentially delete session on server if that's more secure
        else{
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
            } 
        }                                                                        
        pgDisconnect($dbConn);
        return $valid_session;
    }

    function log_in_from_session($cookie_containing_session){
        $dbConn = pgConnect();
        $query = ("SELECT * FROM sessions WHERE session_id='$cookie_containing_session'");
        $results = pgQuery($dbConn, $query);
        $tuple = pg_fetch_assoc($results);
        if(count($tuple) == 0) return; //remove cookies?
        else $session_id = $tuple['session_id'];
        $query = ("SELECT user_name from session_lookup WHERE session_id='$session_id'");
        $results = pgQuery($dbConn, $query);
        $row = pg_fetch_assoc($results);
        pgDisconnect($dbConn);
        //die(var_dump($row['user_name']));
        recreate_session($row['user_name']);
    }
    
    function get_username_from_sessions($cookie_containing_session){
        $dbConn = pgConnect();
        $query = ("SELECT * FROM sessions WHERE session_id='$cookie_containing_session'");
        $results = pgQuery($dbConn, $query);
        $tuple = pg_fetch_assoc($results);
        if(count($tuple) == 0) return; //remove cookies?
        else $session_id = $tuple['session_id'];
        $query = ("SELECT user_name from session_lookup WHERE session_id='$session_id'");
        $results = pgQuery($dbConn, $query);
        $row = pg_fetch_assoc($results);
        pgDisconnect($dbConn);
        //die(var_dump($row['user_name']));
        return ($row['user_name']); 
    }

?>