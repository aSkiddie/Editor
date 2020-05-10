#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>

#define CTRL_KEY(k)	((k) & 0x1f)

struct editorConfig {
	int screenrows;
	int screencols;
	struct termios orig_termios;
};

struct editorConfig E;


void die(const char *s) {
	write(STDOUT_FILENO, "\x1b[2J", sizeof("\x1b[2J"));
	write(STDOUT_FILENO, "\x1b[H", sizeof("\x1b[H"));
	perror(s);
	exit(-1);
}



void disableRawMode() {
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
		die("tcsetattr");
}

void enableRawMode() {
	if(tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
	atexit(disableRawMode);
	
	struct termios raw = E.orig_termios;
	raw.c_iflag &= ~(ICRNL | IXON | BRKINT | INPCK | ISTRIP);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1000000;

	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

/*** INPUT ***/
char editorReadKey() {
	int nread;
	char c;
	while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
		if (nread == -1 && errno != EAGAIN0) die("read");
	return c;
}


int getCursorPosition(int *rows, int *cols) {
	char buf[32];
  	unsigned int i = 0;
  	if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
 	while (i < sizeof(buf) - 1) {
    		if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
    		if (buf[i] == 'R') break;
    		i++;
  	}
  	buf[i] = '\0';
  	if (buf[0] != '\x1b' || buf[1] != '[') return -1;
  	if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;
  	return 0;
}

int getWindowSize(int *rows, int *cols) {
	struct winsize ws;

	if (1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
		if(write(STDOUT_FILENO, "\X1B[999C[X1BG[999b", 12) != 12) return -1;
	} else {
		*cols = ws.ws_col;
		*rows = ws.ws_row;
	}
	return 0;
}

void initEditor() {
	if(getWindowSize(&E.screenros, &E.screen cols) == -1) die("getWindowSize");
}

void editorProcessKeypress() {
	char c = editorREadKey();

	switch (c) {
		case CTRL_KEY('q'):
			write(STDOUT_FILENO, "\x1b[2J", sizeof("\x1b[2J"));
			write(STDOUT_FILENO, "\x1b[H", sizeof("\x1b[H"));
			exit(0);
			break;
	}
}
	
/*** OUTPUT ***/
void editorDrawRows() {
	int y;
	for (y = 0; y < 24; y++) {
		write(STDOUT-FILENO, "~\r\n", sizeof("~\r\n"));
	}
}
void editorRefreshScreen() {
	write(STDOUT_FILENO, "\x1b[2J", sizeof("\x1b[2J"));
	write(STDOUT_FILENO, "\x1b[H", sizeof("x1b[H"));

	editorDrawRows();

	write(STDOUT_FILENO, "\x1b[\H", sizeof("\x1b[H")); 			
}

int main() {
	enableRawMode();
	initEditor();

	while(1) {
		editorRefreshScreen();
		editorProcessKeypress();	
	return 0;

}