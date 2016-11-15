<?php

    include "db_man.php";

    // Verify user account name is unique id
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

    // Verify user login info
    function verify_login_info($email, $pword) {
        $verified = false;        
        $query = "SELECT * FROM user_profiles WHERE user_name = '$email'";
        
        // Connecting to db
        $dbConn = pgConnect();
        
        // Checking for existing user account with same email
        $result = pgQuery($dbConn, $query);
        while($row = pgFetchAssoc($result)) {
            if($row['user_name'] === $email && $row['user_password'] === $pword) {
                $verified = true;
                break;
            }
        }
        
        // Disconnecting from db
        pgDisconnect($dbConn);
                
        return $verified;
    }

    // Create new record in database
    function store_data($email, $pword, $fname, $lname, $ipaddr) {
        $query = "INSERT INTO user_profiles (user_name, user_password, first_name, last_name, last_login, last_ipaddr) VALUES ('$email', '$pword', '$fname', '$lname', 'now', '$ipaddr')";
        
        $dbConn = pgConnect();
        pgQuery($dbConn, $query);
        pgDisconnect($dbConn);
        
        // Note: Can check for error cases eventually
        return true;
    }

    // Returns all user facing data except password in an associative array
    function get_user_info($email) {
        $query = "SELECT user_name, first_name, last_name FROM user_profiles where user_name = '$email'";
        
        $dbConn = pgConnect();
        $results = pgQuery($dbConn, $query);
        pgDisconnect($dbConn);
        
        return pgFetchAssoc($results);
    }

    // Returns associative array containing user_password
    function get_password($email) {
        $query = "SELECT user_password FROM user_profiles where user_name = '$email'";
            
        $dbConn = pgConnect();
        $results = pgQuery($dbConn, $query);
        pgDisconnect($dbConn);
        
        return pgFetchAssoc($results);        
    }

    // Returns an associative array containing last_login and last_ipaddr
    function get_login_info($email) {
        $query = "SELECT last_login, last_ipaddr FROM user_profiles where user_name = '$email'";
            
        $dbConn = pgConnect();
        $results = pgQuery($dbConn, $query);
        pgDisconnect($dbConn);
        
        return pgFetchAssoc($results);        
    }

?>