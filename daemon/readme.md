# Task 6. Daemon-backup program
#### Description of the program
This program takes two parameters: the path to the source directory and to the backup directory.<br>It recursively scans the source directory and makes copies of all files that have not been copied before or that have been modified since the last backup. 
<br>After copying each file, gzip is called.
<br>There is controller for daemon
<br>It takes one or two parameters
* ./con p - printing daemon pid
* ./con k - terminating daemon
* ./con d /path - changing backup directory

#### Compiling
	gcc main.c & gcc controller.c -o con
#### Running
	./a.out /origin /backup
