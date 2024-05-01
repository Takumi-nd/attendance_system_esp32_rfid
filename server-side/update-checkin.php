<?php

$servername = "your_server_name";
$dbname = "your_database_name";
$username = "your_user_database_name";
$password = "your_databse_password";

$api_key_value = "tPmAT5Ab3j7F9";
$api_key = $cardid = "";

$api_key = test_input($_POST["api_key"]);
if($api_key == $api_key_value) {
    $cardid = test_input($_POST["cardid"]);
    
    $conn = new mysqli($servername, $username, $password, $dbname);

    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }
    
    $sql = "SELECT checkin FROM rfidInfo WHERE cardid = '$cardid'";
    $result = $conn->query($sql);
    
    if ($result->num_rows > 0) {
        $row = $result->fetch_assoc();
        $currentCheckin = $row['checkin'];
    
        // Calculate new checkin value (increment and reset if necessary)
        $newCheckin = ($currentCheckin + 1) % 4; // Increment and reset at 4 (0, 1, 2, 3, 4)
    
        // Update checkin value in the database
        $updateSql = "UPDATE rfidInfo SET checkin = $newCheckin WHERE cardid = '$cardid'";
        if ($conn->query($updateSql) === TRUE) {
            echo "Checkin successfully";
        } else {
            echo "Error checkin: " . $conn->error;
        }
    } else {
        echo "$cardid not found";
    }
    
    // Close connection
    $conn->close();
} else {
    echo "Wrong API Key provided.";
}

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}
?>
