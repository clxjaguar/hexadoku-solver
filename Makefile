COMPILER = gcc
CCFLAGS = -Wall
.PHONY: rebuild clean mrproper

hexadoku: hexadoku.o
	@echo "*** Linking all main objects files..."
	@gcc hexadoku.o -o hexadoku -lncurses

hexadoku.o: hexadoku.c
	@echo "*** Compiling hexadoku.o"
	@${COMPILER} ${CCFLAGS} -c hexadoku.c -o hexadoku.o

clean:
	@echo "*** Removing executable."
	@rm hexadoku

mrproper: clean
	@echo "*** Removing objects..."
	@rm *.o

rebuild: mrproper hexadoku
