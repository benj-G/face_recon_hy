<?php

    function verify_login_info($email, $pword) {   
        $verified = false;
        $query = "SELECT * FROM user_profiles WHERE user_name = '$email'";
        
        $dbConn = pgConnect();
        
        $result = pgQuery($dbConn, $query);
        while($row = pgFetchAssoc($result)) {
            if($row['user_name'] === $email && $row['password'] === $pword) {
                $verified = true;
                break;
            }
        }

        pgDisconnect($dbConn);
        
        return $verified;
    }

    // Note: If USER_NAME is a key, we may be able to throw 
    //       an error after failed insert instead of performing this check
    //       Otherwise, this works!
    function is_unique_user($email) {
        $unique = false;
        $query = "SELECT * FROM user_profiles WHERE user_name = '$email'";
        
        $dbConn = pgConnect();
        
        // Note: This check could probably be stronger
        if(pgFetchAssoc(pgQuery($dbConn, $query))['user_name'] == "") {
            $unique = true;
        }

        pgDisconnect($dbConn);
        
        return $unique;
    }

    // Creating a new record in database
    function store_data($email, $pword, $fname, $lname, $ipaddr) {
        $query = "INSERT INTO user_profiles(user_name, user_password, first_name, last_name, last_login, last_ipaddr) VALUES ('$email', '$pword', '$fname', '$lname', 'now', '$ipaddr')";
        
        $dbConn = pgConnect();
        $result = pgQuery($dbConn, $query);
        if(!$result){
            $p_error = pg_last_error($dbConn);
            echo($p_error);
            die();
        }

        pgDisconnect($dbConn);
        
        // Note: Can check for error cases eventually
        return true;
    }

?>