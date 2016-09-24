<?php

    include "post_utils.php";
    include "db_man.php";
    include "db_utils.php";

    $email = $pword = "";
    $errors = false;

    if($_SERVER["REQUEST_METHOD"] == "POST") {

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

        // Display error or redirect to user account page
        if(verify_login_info($email, $pword)) {
            // Note: Create a session here
          header("Location: ../account.html");
        } else {
            // Send errors to client
            echo("Incorrect user and/or password information.");
            // header("Location: ../login.html");
        }

    } else {
        // Serve form submission error page
        header("Location: ../login.html");
        // error: "Submission failed. Please try again."
    }

?>