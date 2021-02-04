# Class Task 4 Feb 2021
#### Description of the program

There are two programs: server and client.<br>
Client accepts commands from stdin and then sends it to server.<br>
After sending client program returns 0.<br>
Server accepts string and recognises two types of commands: print [string] and exit.<br>
If he accepts print then he prints [string] into stdout.<br>
If he accepts exit then he deletes socket file and then shuts down.

#### Compiling
	gcc client.c -o client
	gcc server.c -o server
#### Running
	./server
	
	./client
	print Hello World!
	exit
