/* HEXADOKU SOLVER
 * clx.freeshell.org - January 2015
 * gcc -Wall -lncurses hexadoku.c -o hexadoku && ./hexadoku file.txt */

#include <stdio.h>  // printf()
#include <stdlib.h> // exit()
#include <curses.h> // apt-get install ncurses-dev
#include <string.h> // memcpy()

// hexadokues
#define OFFSETSYMBOLS      0
#define NBROFSYMBOLS      16
#define FIELDSIZEX        16
#define FIELDSIZEY        16
#define FIELDBLOCKSIZEX    4
#define FIELDBLOCKSIZEY    4

/*
// sudokues
#define OFFSETSYMBOLS      1
#define NBROFSYMBOLS       9
#define FIELDSIZEX         9
#define FIELDSIZEY         9
#define FIELDBLOCKSIZEX    3
#define FIELDBLOCKSIZEY    3
*/

/*
// sixdokues
#define OFFSETSYMBOLS      1
#define NBROFSYMBOLS       6
#define FIELDSIZEX         6
#define FIELDSIZEY         6
#define FIELDBLOCKSIZEX    2
#define FIELDBLOCKSIZEY    3
*/

/*
// fourdokues
#define OFFSETSYMBOLS      1
#define NBROFSYMBOLS       4
#define FIELDSIZEX         4
#define FIELDSIZEY         4
#define FIELDBLOCKSIZEX    2
#define FIELDBLOCKSIZEY    2
*/

// defines for display
#define ARRAY_VAL_X 3
#define ARRAY_VAL_Y 4
#define ARRAY_POS_X 30
#define ARRAY_POS_Y 4
#define GUESSES_LIST_X 57
#define GUESSES_LIST_Y 4
#define GUESSES_LIST_COLS 3
#define UNKNOW_VALUE 0xFF

typedef struct {
	unsigned char value;
	unsigned char possibilities;
	unsigned char valueispossible[NBROFSYMBOLS];
	unsigned char isresult;
} tarray;

void goto_xy_array(int x, int y){
	move(y+y/FIELDBLOCKSIZEY+ARRAY_VAL_Y, x+(x/FIELDBLOCKSIZEX)*2+ARRAY_VAL_X);
}

void goto_xy_possibilities(int x, int y){
	move(y+y/FIELDBLOCKSIZEY+ARRAY_POS_Y, x+(x/FIELDBLOCKSIZEX)*2+ARRAY_POS_X);
}

void display_nocurses(tarray array[FIELDSIZEX][FIELDSIZEY]){
	int x, y;
	for (y=0; y<FIELDSIZEY; y++){
		if (y%FIELDBLOCKSIZEY==0) { printf("\n"); }
		for (x=0; x<FIELDSIZEX; x++){
			if (x%FIELDBLOCKSIZEX==0) { printf("  "); }
			if (array[x][y].value==UNKNOW_VALUE) {
				printf(".");
			}
			else {
				printf("%X", array[x][y].value+OFFSETSYMBOLS);
			}
		}
		printf("\n");
	}
	printf("\n");
}

void display_curses(tarray array[FIELDSIZEX][FIELDSIZEY]){
	int x, y;
	for (y=0; y<FIELDSIZEY; y++){
		for (x=0; x<FIELDSIZEX; x++){
			goto_xy_array(x, y);
			if (array[x][y].value==UNKNOW_VALUE) {
				printw(".");
			}
			else {
				printw("%X", array[x][y].value+OFFSETSYMBOLS);
			}
			goto_xy_possibilities(x, y);
			if (array[x][y].possibilities == 0){
				printw(".");
			}
			else if (array[x][y].possibilities < NBROFSYMBOLS){
				printw("%X", array[x][y].possibilities);
			}
			else {
				printw("?");
			}
		}
		refresh();
	}
}

int curses_init(void){
	initscr(); // start curses mode, LINES and ROWS not valids before
	cbreak();  // line input buffering disabled ("raw" mode)
	noecho();  // curses call set to no echoing
	curs_set(0);
	refresh();
	timeout(10);
	if (ARRAY_VAL_Y>3) {
		move(ARRAY_VAL_Y-3, ARRAY_VAL_X); printw("Values array");
		move(ARRAY_VAL_Y-2, ARRAY_VAL_X); printw("------------");
	}
	if (ARRAY_POS_Y>3) {
		move(ARRAY_POS_Y-3, ARRAY_POS_X); printw("Remaining possibilities");
		move(ARRAY_POS_Y-2, ARRAY_POS_X); printw("-----------------------");
	}
	return 0;
}

int hexadoku_open(tarray array[FIELDSIZEX][FIELDSIZEY], char *filename){
	FILE *fd;
	char buf[30];
	int x, y, i, j;

	fd = fopen(filename, "r");
	if (!fd) {
		perror(filename);
		exit(-2);
	}
	y = 0;
	while(fgets(buf, sizeof(buf), fd)){
		if (y>=FIELDSIZEY) { break; }
		i = 0;
		for (x=0; x<FIELDSIZEX;){
			array[x][y].possibilities = 0;
			array[x][y].isresult = 0;
			if (!buf[i]){ break; }
			else if (buf[i]=='\n' || buf[i]=='\r'){ break; }
			else if (buf[i]==' '){ i++; continue; }
			else if (buf[i]>='0' && buf[i]<='9') { array[x][y].value = (buf[i++]-'0'); }
			else if (buf[i]>='A' && buf[i]<='F') { array[x][y].value = (buf[i++]-'A')+0xa; }
			else if (buf[i]>='a' && buf[i]<='f') { array[x][y].value = (buf[i++]-'a')+0xa; }
			else { // unknow
				array[x][y].value = UNKNOW_VALUE;
				for (j=0; j<NBROFSYMBOLS; j++){ array[x][y].valueispossible[j] = 1; }
				array[x][y].possibilities = 0xFF;
				if (buf[i] == '?') { array[x][y].isresult = 1; }
				i++;
			}
			if (array[x][y].value != UNKNOW_VALUE){
				if (array[x][y].value >= OFFSETSYMBOLS){
					array[x][y].value-= OFFSETSYMBOLS;
					if (array[x][y].value >= NBROFSYMBOLS){ array[x][y].value = UNKNOW_VALUE; }
					else {
						for (j=0; j<NBROFSYMBOLS; j++){ array[x][y].valueispossible[j] = 0; }
						x++;
					}
				}
			}
			else {
				x++;
			}
		}
		if (x==FIELDSIZEX) { y++; }
	}
	fclose(fd);

	if (y<FIELDSIZEY) {
		fprintf(stderr, "%s: %s\n", filename, "Sorry, file is not valid!");
		exit(-3);
	}
	return 0;
}

char *hexadoku_getresult(tarray array[FIELDSIZEX][FIELDSIZEY], char *buf, size_t bufsize){
	int x, y, i=0;
	for (y=0; y<FIELDSIZEY; y++){
		for (x=0; x<FIELDSIZEX; x++){
			if (array[x][y].isresult) {
				if (array[x][y].value != UNKNOW_VALUE) {
					sprintf(&buf[i++], "%X", array[x][y].value);
				}
				else {
					sprintf(&buf[i++], "?");
				}
				if (i>=bufsize) { buf[bufsize-1] = '\0'; return buf; }
			}
		}
	}
	buf[i] = '\0';
	return buf;
}

int skip_verbosity=1;
int first_pass=1;

int hexadoku_solve(tarray array[FIELDSIZEX][FIELDSIZEY], int depth){
	tarray array_new[FIELDSIZEX][FIELDSIZEY];
	int x, y, i, j, val, willhavetoguess, lessx, lessy, lessp, finished;

	for(;;){
		willhavetoguess = 1; finished=1;
		lessx = 0; lessy = 0; lessp = 255;
		display_curses(array);

		for(x=0; x<FIELDSIZEX; x++){
			for(y=0; y<FIELDSIZEY; y++){
				if (array[x][y].value != UNKNOW_VALUE) { continue; } // know value
				finished = 0; // as there is at least one unknow value.

				// checking others in the same block
				for(i=(x/FIELDBLOCKSIZEX)*FIELDBLOCKSIZEX; i<(x/FIELDBLOCKSIZEX)*FIELDBLOCKSIZEX+FIELDBLOCKSIZEX; i++){
					for(j=(y/FIELDBLOCKSIZEY)*FIELDBLOCKSIZEY; j<(y/FIELDBLOCKSIZEY)*FIELDBLOCKSIZEY+FIELDBLOCKSIZEY; j++){
						if (i==x && j==y) { continue; }
						val = array[i][j].value;
						if (val != UNKNOW_VALUE) {
							if (array[x][y].valueispossible[val]){
								array[x][y].valueispossible[val] = 0;
								willhavetoguess = 0;
								if (skip_verbosity<1) {
									timeout(50); goto_xy_array(i, j); curs_set(1); refresh(); if (getch() != ERR) { skip_verbosity++; } curs_set(0);
								}
							}
						}
					}
				}

				// checking others vertically (same x)
				for(j=0; j<FIELDSIZEY; j++){
					if (j==y) { continue; }
					val = array[x][j].value;
					if (val != UNKNOW_VALUE) {
						if (array[x][y].valueispossible[val]){
							array[x][y].valueispossible[val] = 0;
							willhavetoguess = 0;
							if (skip_verbosity<1) {
								timeout(50); goto_xy_array(x, j); curs_set(1); refresh(); if (getch() != ERR) { skip_verbosity++; } curs_set(0);
							}
						}
					}
				}

				// checking others horizontally (same y)
				for(i=0; i<FIELDSIZEX; i++){
					if (i==x) { continue; }
					val = array[i][y].value;
					if (val != UNKNOW_VALUE) {
						if (array[x][y].valueispossible[val]){
							array[x][y].valueispossible[val] = 0;
							willhavetoguess = 0;
							if (skip_verbosity<1) {
								timeout(50); goto_xy_array(i, y); curs_set(1); refresh(); if (getch() != ERR) { skip_verbosity++; } curs_set(0);
							}
						}
					}
				}

				// now.. do we only have one solution for this cell ?
				val=-1; j=0;
				for(i=0; i<NBROFSYMBOLS; i++){
					if (array[x][y].valueispossible[i]){
						if (!j++){ val = i; }
					}
				}
				if (array[x][y].possibilities != j) { // update the number of possibilities remaining for that cell
					willhavetoguess = 0;
					array[x][y].possibilities = j;
					if (skip_verbosity<2) { display_curses(array); goto_xy_possibilities(x, y); curs_set(1); timeout(100); if (getch() != ERR) { skip_verbosity++; } timeout(20); curs_set(0); }
				}
				if (j==1){ // only one remaining! so we know its value!
					willhavetoguess = 0;
					array[x][y].value = val;
					array[x][y].possibilities = 0;
					for (val=0; val<NBROFSYMBOLS; val++){ array[x][y].valueispossible[val] = 0; }
					if (skip_verbosity<3){ display_curses(array); goto_xy_possibilities(x, y);  curs_set(1); timeout(100); if (getch() != ERR) { skip_verbosity++; } timeout(20); curs_set(0); }
				}
				else {
					if (j < lessp) { lessp = j; lessx = x; lessy = y; }
					curs_set(0);
				}
			} // "y" loop end
		} // "x" loop end

		// now exclusive deductions (should speed up resolutions)
		if (willhavetoguess){
			for(x=0; x<FIELDSIZEX; x++){
				for(y=0; y<FIELDSIZEY; y++){{
					int sym; int onlyonefound;
					if (array[x][y].value != UNKNOW_VALUE) { continue; } // know value

					// for every candidate
					for (sym=0; sym<NBROFSYMBOLS; sym++){
						if (!array[x][y].valueispossible[sym]) { continue; }

						// checking others in the same block
						onlyonefound=1;
						for(i=(x/FIELDBLOCKSIZEX)*FIELDBLOCKSIZEX; i<(x/FIELDBLOCKSIZEX)*FIELDBLOCKSIZEX+FIELDBLOCKSIZEX; i++){
							for(j=(y/FIELDBLOCKSIZEY)*FIELDBLOCKSIZEY; j<(y/FIELDBLOCKSIZEY)*FIELDBLOCKSIZEY+FIELDBLOCKSIZEY; j++){
								if (i==x && j==y) { continue; }
								if (array[i][j].value == sym){
									onlyonefound=0;
								}
								else if (array[i][j].valueispossible[sym]) {
									onlyonefound=0;
								}
							}
						}

						// checking others vertically (same x)
						if (!onlyonefound){
							for(j=0; j<FIELDSIZEY; j++){
								if (j==y) { continue; }

								if (array[x][j].value == sym){
									onlyonefound=0;
								}
								else if (array[x][j].valueispossible[sym]) {
									onlyonefound=0;
								}
							}
						}

						// checking others horizontally (same y)
						if (!onlyonefound){
							for(i=0; i<FIELDSIZEX; i++){
								if (i==x) { continue; }
								if (array[i][y].value == sym){
									onlyonefound=0;
								}
								else if (array[i][y].valueispossible[sym]) {
									onlyonefound=0;
								}
							}
						}

						if (onlyonefound){
							if (skip_verbosity<5){
								timeout(500); goto_xy_possibilities(x, y); curs_set(1); refresh(); if (getch() != ERR) { skip_verbosity++; } curs_set(0); display_curses(array);
							}
							array[x][y].value = sym;
							array[x][y].possibilities=0;
							willhavetoguess=0;
							for (val=0; val<NBROFSYMBOLS; val++){ array[x][y].valueispossible[val] = 0; }
							display_curses(array);
							break;
						}
					}
				}}
			}
		}

		if (first_pass && skip_verbosity==1){
			first_pass=0;
			skip_verbosity=0;
		}

		refresh();
		if (finished) {
			timeout(10000); getch();
			return 0;
		}


		if (willhavetoguess) {
			move(GUESSES_LIST_Y-3, GUESSES_LIST_X); printw("List of guesses");
			move(GUESSES_LIST_Y-2, GUESSES_LIST_X); printw("---------------");
			// nothing new this turn. trying to write a number...
			for(i=0; i<NBROFSYMBOLS; i++){ // checking among possibles values
				if (array[lessx][lessy].value != UNKNOW_VALUE) { continue; }
				if (array[lessx][lessy].valueispossible[i]){ // that value "i" is yet possible
					// making backup because...
					memcpy(array_new, array, sizeof(tarray[FIELDSIZEX][FIELDSIZEY]));

					// what we're doing now is a terrible thing...
					array_new[lessx][lessy].value = i;
					array_new[lessx][lessy].possibilities = 0;
					for (val=0; val<NBROFSYMBOLS; val++){ array_new[lessx][lessy].valueispossible[val] = 0; }

					// displaying
					move(GUESSES_LIST_Y+depth/GUESSES_LIST_COLS, GUESSES_LIST_X+(depth%GUESSES_LIST_COLS)*10);
					printw("(%d,%d)=%X ", lessx, lessy, i+OFFSETSYMBOLS);

					if (skip_verbosity<4){ // animate on screen to show what we're doing now
						timeout(100);
						for(j=0; j<5; j++){
							curs_set(1);
							goto_xy_array(lessx, lessy); refresh(); if (getch() != ERR) { skip_verbosity++; break; }
							goto_xy_possibilities(lessx, lessy); refresh(); if (getch() != ERR) { skip_verbosity++; break; }
							curs_set(0);
						}
						timeout(10);
					}
					if (!hexadoku_solve(array_new, depth+1)) {
						// return the first solution we find.
						memcpy(array, array_new, sizeof(tarray[FIELDSIZEX][FIELDSIZEY]));
						return 0;
					}
					willhavetoguess = 0;
				}
			}
			if (i==NBROFSYMBOLS){ // impossible to guess more : end of branch.
				move(GUESSES_LIST_Y+depth/GUESSES_LIST_COLS, GUESSES_LIST_X+(depth%GUESSES_LIST_COLS)*10);
				printw("                ", lessx, lessy, i);
				return -1;
			}
		}
	} // main loop end
}

int main(int argc, char **argv) {
	tarray array[FIELDSIZEX][FIELDSIZEY];

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <hexadoku_file>\n", argv[0]);
		return -1;
	}

	/* reading file */
	hexadoku_open(array, argv[1]);
	display_nocurses(array);

	/* initialyzing curses */
	curses_init();

	/* working! */
	hexadoku_solve(array, 0);

	/* bye! */
	endwin();
	display_nocurses(array);

	{
		char buf[200];
		hexadoku_getresult(array, buf, sizeof(buf));
		printf("Result: %s\n", buf);
	}

	return 0;
}
