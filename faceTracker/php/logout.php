<?php

    include "session_utils.php";

    ///////////
    // Script
    ////////

    // Ending client session
    end_session();

    // Redirect them to the landing page
    header("Location: ../index.html");
    
    // Exit script
    exit();
?>