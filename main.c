
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <time.h>
#include <termios.h>
#include <signal.h>

#include <sys/ioctl.h>

#define FG1        37
#define FG2        30
#define BG         39
#define DEFAULT    37
#define WARN       33
#define SCREAM     31
#define DEC        32

typedef unsigned char bool;

typedef union {
	struct {
		bool _1 : 1;
		bool _2 : 1;
		bool _3 : 1;
		bool _4 : 1;
		bool _5 : 1;
		bool _6 : 1;
		bool _7 : 1;
		bool _8 : 1;
	} d;
	unsigned char n;
} bin;

typedef struct {
	enum {
		BINTODEC,
		DECTOBIN
	} type;
	bin data;
} line;

#define b_rand() ((bin)(bool)(rand() % 256))

void b_print(bin b) {
	printf("%d: %d %d %d %d %d %d %d %d\n", b.n,
		b.d._8, b.d._7, b.d._6, b.d._5, b.d._4, b.d._3, b.d._2, b.d._1
	);
}

line          lines[15];
unsigned int  lineslen = 0;
char          inp[9];
unsigned char inpbin = 0;
unsigned int  inpdec = 0;
unsigned char inplength = 0;
unsigned int  score = 0;
unsigned int  level = 0;
unsigned int  linecount = 0;
unsigned int  theme = DEFAULT;

struct termios term, restore;

void __attribute__((__noreturn__)) end() {
	// restore terminal settings
	tcsetattr(0, TCSANOW, &restore); // restore terminal state
	static struct winsize _size;
	ioctl(1, TIOCGWINSZ, &_size);
	#define W _size.ws_col
	#define H _size.ws_row	
	printf("\n\n\x1b[0m\x1b[?25h\x1b[%u;0HGG!\nYour Score was %u (level %u)\n", H, score, level + 1);
	// bye bye
	exit(0);
	#undef W
	#undef H
}

void l_add() {
	if (lineslen >= 15) // die
		end();
	static unsigned int diff;
	lines[lineslen].data.n = 0;
	if      (level > 10) diff = 7;
	else if (level >  8) diff = 6;
	else if (level >  6) diff = 5;
	else if (level >  4) diff = 4;
	else if (level >  2) diff = 3;
	else                 diff = 2;
	for (unsigned int _ = 0; _ < diff; ++_)
		lines[lineslen].data.n |= 1 << (rand() % 7);
	lines[lineslen].type = rand() > RAND_MAX / 2 ? BINTODEC : DECTOBIN;
	++lineslen;
}

void l_check() {
	static bool flag;
	flag = 0;
	for (unsigned int i = 0; i < lineslen; ++i) {
		//printf("%d == %d or %d\n", lines[i].data.n, inpdec, inpbin);
		if (
			(lines[i].type == BINTODEC && lines[i].data.n == (unsigned char)inpdec) ||
			(lines[i].type == DECTOBIN && lines[i].data.n == inpbin)
		) {
			score += lines[i].data.n + 100;
			--lineslen;
			for (unsigned int m = i; m < lineslen; ++m) {
				lines[m] = lines[m + 1];
			}
			flag = 1;
			++linecount;
		}
	}
	//lines[1238123].data.n = 123;
	if (flag) {
		if (lineslen == 0) {
			for (unsigned int _ = 0; _ < 5; ++_) l_add();
			++level;
		}
		inpbin    = 0;
		inpdec    = 0;
		inplength = 0;
	}
}

void l_print() {
	if      (lineslen == 15) theme = SCREAM;
	else if (lineslen == 14) theme = WARN;
	else                     theme = DEFAULT;
	static struct winsize _size;
	ioctl(1, TIOCGWINSZ, &_size);
	#define W _size.ws_col
	#define H _size.ws_row
	#define WW (5 * 9 + 10)
	if (W < WW) {
		printf("Screen width is too small!\n");
		return;
	}
	if (H < 20) {
		printf("Screen height is too small!\n");
		return;
	}
	printf("\x1b[%u;%um\x1b[2J\x1b[2;3H\x1b[1mLevel\x1b[22m\n  %u\n\n \x1b[1m Score\x1b[22m\n  %u\n\n\x1b[1m  Lines\x1b[22m\n  %u\x1b[1;%uH ┌─────┬─────┬─────┤ \x1b[1mThe Binary Game\x1b[22m ├─────┬─────┬─────┐\n", theme, BG + 10, level + 1, score, linecount, (W - WW) / 2);
	for (unsigned int __ = 0; __ < H - lineslen - 4; ++__) {
		printf("\x1b[%uC│     │     │     │     │     │     │     │     │     │\n", (W - WW) / 2);
	}
	unsigned int col;
	unsigned int val;
	for (unsigned int i = 0; i < lineslen; ++i) {
		printf("\x1b[%uC", (W - WW) / 2);
		if (lines[i].type == BINTODEC) {
			printf("│     ");
			col = 128;
			for (unsigned int _ = 0; _ < 8; ++_) {
				val = lines[i].data.n & col;
				if (val == 0) {
					if (col == 128) printf("│");
					printf("  0  ");
					if (col == 1 || (lines[i].data.n & (col >> 1)) == 0) printf("│");
				} else {//
					if (col == 128 || (lines[i].data.n & (col << 1)) == 0)
						printf("▐\x1b[7m");
					else
						printf("\x1b[7m│");				
					printf("  1  \x1b[27m");
					if (col == 1 || (lines[i].data.n & (col >> 1)) == 0) printf("▋");
				}
				col >>= 1;
			}
			putchar('\n');
		} else {
			printf("\x1b[%um▐\x1b[7m %3u \x1b[27m▋\x1b[%um     │     │     │     │     │     │     │     │\n", DEC, lines[i].data.n, theme);
		}
	}
	//printf("\x1b[%u;%um\x1b[%uC  DEC   128    64    32    16     8     4     2     1  \x1b[1m\x1b[%uC", theme + 10, FG2, (W - WW) / 2, W - WW / 2 - inplength);
	printf("\x1b[%u;%um\x1b[%uC\x1b[1K  DEC   128    64    32    16     8     4     2     1  \x1b[1m\x1b[0K\n\x1b[%uC\x1b[1K", theme + 10, FG2, (W - WW) / 2, W / 2 - inplength);
	for (unsigned int i = 0; i < inplength; ++i) {
		putchar(inp[i]);
		putchar(' ');
	}
	printf("\x1b[0J\x1b[0m");
	#undef W
	#undef H
}

void sighandle() {
	score += 100;
	l_add();
	l_print();
	if (level >= 10)
		alarm(2);
	else
		alarm(7 - level / 2);
}

int main(void) {
	// init rand
	srand(time(NULL));

	// set terminal settings (dont echo characters / queue stdin / ignore ctrl-c )
	tcgetattr(0, &term);
	tcgetattr(0, &restore); // backup the original terminal state to restore later
	term.c_lflag &= ~(ICANON|ECHO|ISIG);
	tcsetattr(0, TCSANOW, &term);
	printf("\x1b[?25l"); // hide cursor
	signal(SIGWINCH, l_print);

	// set up signal handeler
	signal(SIGALRM, sighandle);
	alarm(5);
	
	for (unsigned int _ = 0; _ < 5; ++_) l_add();
	l_print();

	char c;
	while ((c = getchar_unlocked()) != 3) {
		switch (c) {
			case '0' ... '9':
				if (inplength >= 8) break;
				inp[inplength] = c;
				++inplength;
				// calc bin
				inpbin <<= 1;
				if (c == '1') ++inpbin;
				// calc dec
				inpdec *= 10;
				inpdec += c - '0';
				break;
			case 127: // \b
				if (inplength == 0) break;
				--inplength;
				// calc bin
				inpbin >>= 1;
				// calc dec
				inpdec /= 10;
				break;
			case 10: // \n
				inplength = 0;
				inpdec = 0;
				inpbin = 0;
				break;
		}
		l_check();
		l_print();
	}

	end();

}
