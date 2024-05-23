# SIMPLE CHEKIN PROJECT USE ESP32
The project use Arduino Framework to programming for ESP32. Components of the circuit has ESP32 NodeMCU-32S as the client to communicate with server. ESP32 will read id from MIFARE 1K card and make HTTP request to a PHP script to store data into a MySQL database. 000webhost help we manage, edit PHP script, database, ... and hosting website. 

The project has two main tasks. Firstly, checkin function that help we check does the id exist in database, and second is write new data to database. Demo is described later.

My circuit here:
image

## Checkin task
After swiping the card, ESP32 will send the card's ID to database and then verify if the ID exist in it. If the ID is found, the OLED display will show 'Successfully', and the CHECKIN value in databse will increase by 1. Otherwise, it will display 'Not found'. If CHECKIN value reaches 4, this value will be reset.

image found
image not found

## Write new card to databse
After read the card's ID, it will be checked in the database. If the ID is found, another card must be use. Otherwise, we will utilize the Serial Monitor to register a new card.

image
image