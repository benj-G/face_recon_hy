<?php

    function clean_post($data) {
        $data = trim($data);
        $data = stripslashes($data);
        $data = htmlspecialchars($data);
        return $data;
    }

    function is_valid_email($data) {
        return (filter_var($data, FILTER_VALIDATE_EMAIL)) ? true : false;
    }

    function is_valid_password($data) {
        // Validate against password requirements
        return true;
    }

?>