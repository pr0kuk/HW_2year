# Testing transfering file using shared memory

#### Compiling programs

	gcc main.c -lpthread && gcc hand.c -o hand -lpthread
	
#### Running programs<br>

*a.out* should be started first
<br>*a.out* takes as argument name of input file
<br>*hand* takes as argument name of ouput file

	./a.out file
	./hand file.copy
