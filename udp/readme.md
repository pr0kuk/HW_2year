# Class Task 18 Feb 2021
#### Description of the program

There are two programs: server and client.<br>
Client accepts commands from stdin and then sends it to server using UDP Protocol.<br>
Server working on 127.0.0.1:23456<br>
Server supports multiple clients<br>
Client has his own command broadcast, which send !hello! signal to INADDR_BROADCAST, servers answers to it revealing their adresses<br>
Server accepts string and recognises four types of commands: print [string], ls, cd [string] and exit.<br>
If he accepts print then he prints [string] into stdout.<br>
If he accepts ls then he prints<br>
If he accepts cd then he change current directory<br>
If he accepts shell then he started pty<br>
If he accepts quit then he shuts down the process working with this client, program-client returns 0.<br>


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
	broadcast
	shell
	ls
	ps avx
	exit
	quit
