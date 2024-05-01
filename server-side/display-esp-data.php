<!DOCTYPE html>
<html><body>
<h1>RFID CHECK IN APP</h1>
<?php

$servername = "your_server_name";
$dbname = "your_database_name";
$username = "your_user_database_name";
$password = "your_databse_password";

$conn = new mysqli($servername, $username, $password, $dbname);

if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 

$sql = "SELECT cardid, name, phone, checkin, time FROM rfidInfo";

echo '<table cellspacing="5" cellpadding="5">
      <tr> 
        <td>CARDID</td>
        <td>NAME</td> 
        <td>PHONE</td> 
        <td>CHECKIN</td>
        <td>Time</td> 
      </tr>';
 
if ($result = $conn->query($sql)) {
    while ($row = $result->fetch_assoc()) {
        $row_cardid = $row["cardid"];
        $row_name = $row["name"];
        $row_phone = $row["phone"];
        $row_checkin = $row["checkin"];
        $row_time = $row["time"];
        // Uncomment to set timezone to - 1 hour (you can change 1 to any number)
        //$row_reading_time = date("Y-m-d H:i:s", strtotime("$row_reading_time - 1 hours"));
      
        // Uncomment to set timezone to + 4 hours (you can change 4 to any number)
        //$row_reading_time = date("Y-m-d H:i:s", strtotime("$row_reading_time + 4 hours"));
      
        echo '<tr> 
                <td>' . $row_cardid . '</td> 
                <td>' . $row_name . '</td> 
                <td>' . $row_phone . '</td> 
                <td>' . $row_checkin . '</td>
                <td>' . $row_time . '</td> 
              </tr>';
    }
    $result->free();
}

$conn->close();
?> 
</table>
</body>
</html>