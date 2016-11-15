<?php
    
    include "post_utils.php";
    include "db_utils.php";
    include "session_utils.php";
    include "account.php";
    ///////////
    // Script
    ////////

    $email = $pword = $fname = $lname = $ipaddr = "";
    $errors = false;

    if(isset($_POST["submit"]) && $_SERVER["REQUEST_METHOD"] == "POST") {

        // Validating email format
        $email = clean_post($_POST["email"]);
        if(!empty($_POST['email']) || !is_valid_email($email)) {
            $errors = true;
        }

        // Validating password format
        $pword = clean_post($_POST["pword"]);
        if(!empty($_POST['pword']) || !is_valid_password($pword)) {  
            $errors = true;
        }

        $fname = clean_post($_POST["fname"]);
        if(!preg_match("/^[a-zA-Z ]*$/", $fname)) {
            $errors = true;
        }

        $lname = clean_post($_POST["lname"]);
        if(!preg_match("/^[a-zA-Z ]*$/", $lname)) {
            $errors = true;
        }

        // Note: Not working, gives ""
        $ipaddr = filter_var($_SERVER["REMOTE_ADDR"], FILTER_VALIDATE_IP);
        // Display errors or redirect to user account page
        if(is_unique_user($email)) {
            // Storing login info
            store_data($email, $pword, $fname, $lname, $ipaddr);
            save_pass($email, $pword);
            // Creating new client session
            create_session($email);
            
            // Serving user account page
            display_account_page();
        } else {
            echo("An account by that name already exists.");
        }

    } else {
        // Redirect to form submission error page
        //echo(var_dump($_POST));
        header("Location: ../signup.html");
        // error: "Submission failed. Please try again."
    }

    // Exit script
    exit();

?>