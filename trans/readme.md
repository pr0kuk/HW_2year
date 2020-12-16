# Translation a file using signals
#### Compiling

	gcc main.c & gcc handler.c -o hand
	
#### Running

	./hand
	./a.out [file_name] [hand_pid]
	
#### Description

This program moves the file specified by the first argument for "a.out" to the file.copy located in the directory of the "hand" program. <br>
First, the "hand" starts and prints its pid, then the out is started. "a.out" sends a SIGCONT welcome signal to the "hand" so that the "hand" recognizes the sender and remembers its pid. <br>(note: the hand remembers only the first sender, the rest are ignored and fall into an endless loop).<br>
 The "a.out" program reads sequentially bytes from a file. Splits them into bits and sends SIGUSR1 if bit is 0, and SIGUSR2 if bit is 1.<br> After sending signal "a.out" fall into an endless loop until the SIGUSR1 is received from "hand".<br> The "hand" program receives signals and writes them to an array of 8 numbers. When the array is full, the "hand" glues one byte of 8 bits and writes it to file.copy.<br>
After "a.out" has read all the bytes, it sends a SIGTERM signal indicating the end of the file.<br> 
If one of the programs receives a SIGINT signal, it sends SIGKILL to itself and to the other program

The programms uses following signals
* SIGUSR1 a.out->hand current bit is 0
* SIGUSR1 hand->a.out previous bit is handled, waiting for next..
* SIGUSR2 a.out->hand current bit is 1
* SIGCONT a.out->hand initialising sender
* SIGTERM a.out->hand end of file was reached
* SIGINT  a.out->hand a.out was interrupted
* SIGINT  hand->a.out hand was interrupted
