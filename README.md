# FileExchange
Simple, no config, no install, file exchange tool/utility

A small tool I first made during the summer of 2007, and only recently improved.
I've used it for the last 19 years and in the odd chance it might be useful to anyone, I'm making it available to everybody.

![alt text](https://github.com/tDavidM/FileExchange/blob/main/Screenshot_Server.png "Application in Server mode")
![alt text](https://github.com/tDavidM/FileExchange/blob/main/Screenshot_Client.png "Application in Client mode")

The Server groupbox (left) is the receiving portion, a target directory must be selected to activate it.
The Client groupbox (center) is the sending portion, an IP address must be enter.

Files as well as directories can be dragged & dropped over the program's window to be sent to the server. The button "Send" opens a file dialog that supports multi-select.
All new files and directories will be created at the target location on the server, existing files will be overwritten.
The client will explore recursively the structure of any directories included and reconstruct the relative path on the server at the target location.

The 2 lower progress bars are for individual files, the upper one on the client is for the total.

The Network groupbox (right) is used to set the port (1755 by default as it was open on school network's firewall in 2007), the computer name to be broadcasted, activate encryption and list other instances available on the network.
Although this tool is meant to be used on a LAN (the broadcast/available instances list only work on LAN), it can be use over WAN with the proper firewall config.
The encryption key must be same on both sending and receiving ends otherwise the files will be unreadable on the server.


This project includes software developed by:
-The Rijndael cipher also known as Advanced Encryption Standard (AES) was developed Joan Daemen and Vincent Rijmen.
-The Tiger hash function was designed by Ross Anderson and Eli Biham.
-The SHA-256 cryptographic hash function was designed by the National Security Agency (NSA) and published by the NIST.
-The Argon2 hash function was designed by Alex Biryukov, Daniel Dinu, and Dmitry Khovratovich and implemented by Loup Vaillant in his library (Monocypher).
