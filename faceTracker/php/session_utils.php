<?php

    include "session_man.php";

    // Create new client session if inactive
    function create_session($email) {
        if(!is_session_active()) {
            create_client_session($email); 
        }
    }

    // Verify session is active and valid
    function is_session_valid() {
        return (is_session_active()) ? true : false;
    }

?>