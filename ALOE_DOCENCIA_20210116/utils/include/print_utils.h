/*
 * print_utils.h
 *
 *
 * This file is part of ALOE.
 *
 * ALOE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ALOE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ALOE.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _PRINT_UTILS
#define _PRINT_UTILS

/*http://ascii-table.com/ansi-escape-sequences.php*/
/*http://cboard.cprogramming.com/linux-programming/151274-dividing-terminal.html*/
/*http://www.safe-c.org/manual.html*/

/* Foreground Colors*/
#define T_BLACK   		"\033[22;30m"
#define T_RED     		"\033[22;31m"
#define T_GREEN   		"\033[22;32m"
#define T_BROWN  		"\033[22;33m"
#define T_BLUE    		"\033[22;34m"
#define T_MAGENTA 		"\033[22;35m"
#define T_CYAN    		"\033[22;36m"
#define T_GRAY			"\033[22;37m"

#define T_DARKGRAY   	"\033[01;30m"
#define T_LIGHTRED     	"\033[01;31m"
#define T_LIGHTGREEN    "\033[01;32m"
#define T_YELLOW  		"\033[01;33m"
#define T_LIGHTBLUE  	"\033[01;34m"
#define T_LIGHTMAGENTA  "\033[01;35m"
#define T_LIGHTCYAN  	"\033[01;36m"
#define T_WHITE			"\033[01;37m"

/* Background Colors*/
#define B_BLACK   		"\033[40m"
#define B_RED     		"\033[41m"
#define B_GREEN   		"\033[42m"
#define B_YELLOW  		"\033[43m"
#define B_BLUE    		"\033[44m"
#define B_MAGENTA 		"\033[45m"
#define B_CYAN    		"\033[46m"
#define B_WHITE			"\033[01;47m"


/* Text Attributes*/
#define RESET   		"\033[0m"
#define UNDERLINE    	"\033[4m"
#define NO_UNDERLINE 	"\033[24m"
#define BOLD         	"\033[1m"
#define NO_BOLD 		"\033[22m"
#define BLINK			"\033[5m"
#define REVERSE			"\033[7m"
#define CONCEALED		"\033[8m"

/* ACTIONS*/
#define ERASE_DISPLAY	"033[23"
#define CLEAR()			printf("\033[H\033[J")
#define gotoxy(x,y)		printf("\033[%d;%dH", (x), (y))


#define COLOR(x, atr,fore,back) snprintf(x, sizeof(x), "033[%d;%d;%dm", atr, fore, back)	//Atr: atribute; fore: foreground; back: background
#define COLORA(x, fore,back) snprintf(x, sizeof(x), "\33[38;5;%d;48;5;%dm", fore, back)


/*
In the linux terminal you may use terminal commands to move your cursor, such as

printf("\033[8;5Hhello"); // Move to (8, 5) and output hello

other similar commands:

printf("\033[XA"); // Move up X lines;
printf("\033[XB"); // Move down X lines;
printf("\033[XC"); // Move right X column;
printf("\033[XD"); // Move left X column;
printf("\033[2J"); // Clear screen
*/

/* Print data arrays*/ 
void print_cplx(char *name, _Complex float *in, int datalength, int rowlength);
void print_float(char *name, float *in, int datalength, int rowlength);
void print_int(char *name, int *in, int datalength, int rowlength);
void print_char(char *name, char *in, int datalength, int rowlength);
void print_bit(char *name, char *in, int datalength, int rowlength);
int print_array(char *name, char *datatype, void *in, int datalength, int rowlength);

#endif /*  */
