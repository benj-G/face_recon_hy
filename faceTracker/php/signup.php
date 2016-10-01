<?php
    
    include "post_utils.php";
    include "db_man.php";
    include "db_utils.php";

    function validate_signup() {
        $email = $pword = $fname = $lname = $ipaddr = "";
        $errors = false;

        if ($_SERVER["REQUEST_METHOD"] == "POST") {

            // Validating email format
            $email = clean_post($_POST["email"]);
            if (!empty($_POST['email']) || !is_valid_email($email)) {
                $errors = true;
            }

            // Validating password format
            $pword = clean_post($_POST["pword"]);
            if(!empty($_POST['pword']) || !is_valid_password($pword)) {  
                $errors = true;
            }

            $fname = clean_post($_POST["fname"]);
            if (!preg_match("/^[a-zA-Z ]*$/", $fname)) {
                $errors = true;
            }

            $lname = clean_post($_POST["lname"]);
            if (!preg_match("/^[a-zA-Z ]*$/", $lname)) {
                $errors = true;
            }

            $ipaddr = filter_var($_SERVER["REMOTE_ADDR"], FILTER_VALIDATE_IP);

            // Display errors or redirect to user account page
            if(is_unique_user($email)) {
                // Store login 
                store_data($email, $pword, $fname, $lname, $_SERVER['REMOTE_ADDR']);
                // Note: Create a session here
                header("Location: ../account.html");
            } else {
                // error: "User name already exists."
                echo("An account by that name already exists.");
                // header("Location: ../signup.html");
            }

        } else {
            // Serve form submission error page
            header("Location: ../signup.html");
            // error: "Submission failed. Please try again."
        }
    }

    validate_signup();
    
?>