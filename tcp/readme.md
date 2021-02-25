# Class Task 11 Feb 2021
#### Description of the program

There are two programs: server and client working on TCP Protocol.<br>
Server starts at 127.0.0.1:23456<br>
Client accepts commands from stdin and then sends it to server.<br>
After sending client program returns 0.<br>
Server accepts string and recognises two types of commands: print [string] and exit.<br>
If he accepts print then he prints [string] into stdout.<br>
If he accepts exit then the server shuts down.

#### Compiling
	gcc client.c -o client
	gcc server.c -o server
#### Running
	./server
	
	./client [ip] (127.0.0.1)
	print Hello World!
	exit
