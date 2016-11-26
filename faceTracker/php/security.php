<?php
	//HUGE secuirty problems

	function save_pass($usrname, $passwd){
		$phash;
		$salt_result;
		$result;
		$inphash;
		exec ("cd ../bin/ && openssl rand -base64 6", $salt_result); //need to store this salt with the user?
		$salt = $salt_result[0];
		exec ("cd ../bin/ && openssl passwd -1 -salt $salt $passwd", $phash, $result);
		//	die(var_dump($phash));
		$inphash = substr($phash[0], 12, strlen($phash[0]) - 1);
		//die(var_dump($inphash));
		//exec ('cd ../bin/ && openssl');
		$query = "UPDATE USER_PROFILES SET USER_PASSWORD='$inphash' WHERE USER_NAME='$usrname'; UPDATE USER_PROFILES SET SALT='$salt' WHERE USER_NAME='$usrname'";
		$dbConn = pgConnect();
		pg_query($dbConn, $query);
		pgDisconnect($dbConn);
		return;
	}

	function get_hash($word, $salt){
		$phash; $result;
		exec ("cd ../bin/ && openssl passwd -1 -salt $salt $word", $phash, $result);
		$inphash = substr($phash[0], 12, strlen($phash[0]) - 1);
		return $inphash;
	}
?>