all: eor add mul sub cmp
	gcc -mfpu=neon socketNeon.c trigger.c execNeon.S 	 -o nodebug.out
	gcc -mfpu=neon socketNeon.c trigger.c execNeon.S -DDEBUG -o debug.out
	gcc -mfpu=neon socketNeon.c trigger.c execNeon.S -g	 -o GDBdebug.out

# Recipes for compiling pre-built assembly objs
eor:
	gcc -mfpu=neon neonExecute_INSTR.S -D INSTR=veor.u32 -D WORD=u32 -c -o neonExecute_veor.o


