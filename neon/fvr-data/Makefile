all:
	gcc -mfpu=neon socketNeon.c trigger.c execNeon.S 	 -o nodebug.out
	gcc -mfpu=neon socketNeon.c trigger.c execNeon.S -DDEBUG -o debug.out
	gcc -mfpu=neon socketNeon.c trigger.c execNeon.S -g	 -o GDBdebug.out


