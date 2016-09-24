<?php

    // Note: If time permits, we can thread this module so multiple connections are possible
    //       Just need to watch out for race conditions

    // Open a connection with specified Postgres db
    function pgConnect() {
        // Temp hardcoded connection info
        $host = "host=127.0.0.1";
        $port = "port=5432";
        $dbname = "dbname=login_test";
        $cred = "user=postgres password=postgres";
        
        // Note: Pass PGSQL_CONNECT_FORCE_NEW if multiple connections desired
        $pgConnection = pg_connect("$host $port $dbname $cred");

        if(!$pgConnection) {
            // Failed to connect, do something!
        }
        
        return $pgConnection;
    }
    
    // Request data from connected db using normal Postgres query syntax
    function pgQuery($pgConnection, $query) {
        // Note: Using pg_query() for now, but can expand with 
        //       pg_send_query() for multiple connection systems
        return pg_query($pgConnection, $query);
    }

    // Transform query results into associative array with column names for keys
    function pgFetchAssoc($result) {
        return pg_fetch_assoc($result);
    }

    // Close once connection no longer in use
    function pgDisconnect($pgConnection) {
        while(pg_connection_busy($pgConnection)) {
            // Do nothing while connection is busy
        }
        
        pg_close($pgConnection);
    }

?>
