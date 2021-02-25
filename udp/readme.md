# Class Task 18 Feb 2021
#### Description of the program

There are two programs: server and client.<br>
Client accepts commands from stdin and then sends it to server using UDP Protocol.<br>
Server working on 127.0.0.1:23456<br>
After sending client program returns 0.<br>
Server accepts string and recognises four types of commands: print [string], ls, cd [string] and exit.<br>
If he accepts print then he prints [string] into stdout.<br>
If he accepts ls then he prints 
If he accepts exit then he deletes socket file and then shuts down.

#### Compiling
	gcc client.c -o client
	gcc server.c -o server
#### Running
	./server
	
	./client [ip] (127.0.0.1)
	print Hello World!
	ls
	cd /bin
	ls
	exit
