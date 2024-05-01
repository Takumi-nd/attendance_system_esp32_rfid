<?php

$servername = "your_server_name";
$dbname = "your_database_name";
$username = "your_user_database_name";
$password = "your_databse_password";

$api_key_value = "tPmAT5Ab3j7F9";

$api_key= $cardid = $name = $phone = "";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $api_key = test_input($_POST["api_key"]);
    if($api_key == $api_key_value) {
        $cardid = test_input($_POST["cardid"]);
        $name = test_input($_POST["name"]);
        $phone = test_input($_POST["phone"]);
        
        // Create connection
        $conn = new mysqli($servername, $username, $password, $dbname);
        // Check connection
        if ($conn->connect_error) {
            die("Connection failed: " . $conn->connect_error);
        } 
        
        $sql = "INSERT INTO rfidInfo (cardid, name, phone)
        VALUES ('" . $cardid . "', '" . $name . "', '" . $phone . "')";
        
        if ($conn->query($sql) === TRUE) {
            echo "Insert $cardid successfully";
        } 
        else {
            if ($conn->errno == 1062) { 
                http_response_code(409); 
                echo "$cardid is conflict";
            }
        }
    
        $conn->close();
    }
    else {
        echo "Wrong API Key provided.";
    }
}
else {
    echo "No data posted with HTTP POST.";
}

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}