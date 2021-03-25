# Class Task 18 Mar 2021
#### Description of the program
Default LOG-Path is "/var/log/server.h"<br>


#####There are three LOG-macros
	pr_err (char *)
	pr_warn(char *)
	pr_info(char *)

#####Examples of use:
	INPUT:
	pr_err  ("I'm here!");
	pr_info ("I'm here!");
	pr_warn ("I'm here!");<br>
	OUTPUT:
	[ERR ][Thu Mar 25 14:23:02 2021][25856][log.c:83] I'm here!
	[WARN][Thu Mar 25 14:23:02 2021][25856][log.c:84] I'm here!
	[INFO][Thu Mar 25 14:23:02 2021][25856][log.c:85] I'm here!


#### Compiling
	gcc main.c log.c -o log
	
#### Running
	./log
