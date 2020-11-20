# Directories class tasks
## Printing all file names in input directory

This program takes the path to the directory in arguments.<br>
Output is a list of all files in it.
#### Compiling program

	gcc read.c -o read
	
#### Running program

	./read /bin

## Detecting changes in directory

This program takes the path to the directory in arguments.<br>
This program is running infinitely. To stop it input ^C.<br>
Output is notifies aboout creating/deleting files in directory or moving directory<br>

#### Compiling program

	gcc inotify.c -o inotify
	
#### Running program

	./inotify /bin
