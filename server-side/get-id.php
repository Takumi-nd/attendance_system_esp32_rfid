<?php
$servername = "your_server_name";
$dbname = "your_database_name";
$username = "your_user_database_name";
$password = "your_databse_password";


$conn = new mysqli($servername, $username, $password, $dbname);

if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

if (isset($_GET['cardid'])) {
    $cardid = test_input($_GET['cardid']);

    $sql = "SELECT cardid FROM rfidInfo WHERE CARDID = '$cardid'";
    $result = $conn->query($sql);

    if ($result->num_rows > 0) {
        echo  "$cardid found";
    } else {
        http_response_code(404);
        echo  "$cardid not found";
    }
} else {
    http_response_code(400);
    echo "ID parameter is missing";
}

$conn->close();

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}
