# Description of the test
#### Compile main.c file

	gcc main.c stack.c
	
#### Running the binary file.

If real-time testing is required, run the file with no arguments 

	./a.out
	
If you need to test prepared instructions, then specify the file name in the argument

	./a.out file

#### Instructions for file-mode testing
*(For the real-time mode hints are displayed in stdout)*<br>
The sample is located in the repository.<br>
The first line must indicate the size of the created stack (If the stack has already been created, you must specify the size not exceeding the existing one)<br>
The file must be ended by the *mark_destruct* or *finish* statement

This stack supports following instructions:
*		get_size (get_s)
* 		get_count (get_c)
*		set_size (s) [val] (val == 0 || val == -1)
*		set_size (s) [val] [seconds] [nanoseconds] (val == 1)
* 		push (pu) [input]
* 		pop (po)
* 		print_all (pr)
*		detach_stack (d)
* 		mark_destruct (m)
* 		finish (f)


#### Compile stress test

	gcc test.c stack.c -o test
	
#### Running stress test

	sudo ./test
	
Super user rights are required because of setgid() and killpg().<br>
The test is successful if the ouptut contains an empty stack.
