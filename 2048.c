/*
 ============================================================================
 Name        : 2048.c
 Author      : Maurits van der Schee
 Description : Console version of the game "2048" for GNU/Linux
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

const int8_t SIZE=3;

bool condenseRow(u_int16_t row[SIZE]);

void getColor(u_int16_t value, char *color, size_t length) {
	u_int16_t c = 40;
	if (value > 0) while (value >>= 1) c++;
	snprintf(color,length,"\033[0;41;%dm",c);
}

void drawBoard(u_int16_t board[SIZE][SIZE]) {
	int8_t x,y;
	char color[20], reset[] = "\033[0m";
	printf("\033[2J");

	for (x=0;x<SIZE;x++) {
		printf(" ______");
	}
	printf(" \n");
	for (y=0;y<SIZE;y++) {
		for (x=0;x<SIZE;x++) {
			getColor(board[x][y],color,20);
			printf("%s",color);
			printf("|      ");
			printf("%s",reset);
		}
		printf("|\n");
		for (x=0;x<SIZE;x++) {
			getColor(board[x][y],color,20);
			printf("%s",color);
			if (board[x][y]!=0) {
				char s[7];
				snprintf(s,7,"%u",board[x][y]);
				int8_t t = 6-strlen(s);
				printf("|%*s%s%*s",t-t/2,"",s,t/2,"");
			} else {
				printf("|      ");
			}
			printf("%s",reset);
		}
		printf("|\n");
		for (x=0;x<SIZE;x++) {
			getColor(board[x][y],color,20);
			printf("%s",color);
			printf("|______");
			printf("%s",reset);
		}
		printf("|\n");
	}
	printf("\nPress arrow keys or 'q' to quit\n\n");
}

int8_t rowLength(u_int16_t row[SIZE]) {
	int8_t len;
	len = SIZE;
	while (len>0 && row[len-1]==0) {
		len--;
	}
	return len;
}

void rotateBoard(u_int16_t a[SIZE][SIZE]) {
	int8_t i,j,n=SIZE;
	u_int16_t tmp;
	for (i=0; i<n/2; i++){
		for (j=i; j<n-i-1; j++){
			tmp=a[i][j];
			a[i][j]=a[j][n-i-1];
			a[j][n-i-1]=a[n-i-1][n-j-1];
			a[n-i-1][n-j-1]=a[n-j-1][i];
			a[n-j-1][i]=tmp;
		}
	}
}

bool moveUp(u_int16_t board[SIZE][SIZE]) {
	bool success = false;
	int8_t x;
	for (x=0;x<SIZE;x++) {
		success |= condenseRow(board[x]);
	}
	return success;
}

bool moveLeft(u_int16_t board[SIZE][SIZE]) {
	bool success;
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	return success;
}

bool moveDown(u_int16_t board[SIZE][SIZE]) {
	bool success;
	rotateBoard(board);
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	rotateBoard(board);
	return success;
}

bool moveRight(u_int16_t board[SIZE][SIZE]) {
	bool success;
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	return success;
}

bool findPairDown(u_int16_t board[SIZE][SIZE]) {
	bool success = false;
	int8_t x,y;
	for (x=0;x<SIZE;x++) {
		for (y=0;y<SIZE-1;y++) {
			if (board[x][y]==board[x][y+1]) return true;
		}
	}
	return success;
}

int16_t countEmpty(u_int16_t board[SIZE][SIZE]) {
	int8_t x,y;
	int16_t count=0;
	for (x=0;x<SIZE;x++) {
		for (y=0;y<SIZE;y++) {
			if (board[x][y]==0) {
				count++;
			}
		}
	}
	return count;
}

bool gameEnded(u_int16_t board[SIZE][SIZE]) {
	bool ended = true;
	if (countEmpty(board)>0) return false;
    if (findPairDown(board)) return false;
	rotateBoard(board);
    if (findPairDown(board)) ended = false;
    rotateBoard(board);
    rotateBoard(board);
    rotateBoard(board);
    return ended;
}

void addRandom(u_int16_t board[SIZE][SIZE]) {
	static bool initialized = false;
	int8_t x,y;
	int16_t r,len=0;
	u_int16_t n,list[SIZE*SIZE][2];

	if (!initialized) {
		srand(time(NULL));
		initialized = true;
	}

	for (x=0;x<SIZE;x++) {
		for (y=0;y<SIZE;y++) {
			if (board[x][y]==0) {
				list[len][0]=x;
				list[len][1]=y;
				len++;
			}
		}
	}

	if (len>0) {
		r = rand()%len;
		x = list[r][0];
		y = list[r][1];
		n = (rand()%2+1)*2;
		board[x][y]=n;
	}
}

void setBufferedInput(bool enable) {
	static bool enabled = true;
	static struct termios old;
	struct termios new;

	if (enable && !enabled) {
		// restore the former settings
		tcsetattr(STDIN_FILENO,TCSANOW,&old);
		// set the new state
		enabled = true;
	} else if (!enable && enabled) {
		// get the terminal settings for standard input
		tcgetattr(STDIN_FILENO,&new);
		// we want to keep the old setting to restore them at the end
		old = new;
		// disable canonical mode (buffered i/o) and local echo
		new.c_lflag &=(~ICANON & ~ECHO);
		// set the new settings immediately
		tcsetattr(STDIN_FILENO,TCSANOW,&new);
		// set the new state
		enabled = false;
	}
}

bool shiftRow(u_int16_t row[SIZE],int8_t start,int8_t length) {
	bool success = false;
	int8_t x,i;
	for (x=start;x<length-1;x++) {
    	while (row[x]==0) {
			for (i=x;i<length-1;i++) {
				row[i] = row[i+1];
				row[i+1] = 0;
				length = rowLength(row);
				success = true;
			}
		}
	}
	return success;
}

bool collapseRow(u_int16_t row[SIZE],int8_t x) {
	bool success = false;
	if (row[x] == row[x+1]) {
		row[x] *= 2;
		row[x+1] = 0;
		success = true;
	}
	return success;
}

bool condenseRow(u_int16_t row[SIZE]) {
	bool success = false;
	int8_t x,length;
	length = rowLength(row);
	for (x=0;x<length-1;x++) {
		length = rowLength(row);
		success |= shiftRow(row,x,length);
		length = rowLength(row);
		success |= collapseRow(row,x);
	}
	return success;
}

int main(void) {
	u_int16_t board[SIZE][SIZE];
	char c;
	bool success;

	memset(board,0,sizeof(board));
	addRandom(board);
	addRandom(board);
	drawBoard(board);

	setBufferedInput(false);
	do {
		c=getchar();
		switch(c) {
			case 68: success = moveLeft(board);  break;
			case 67: success = moveRight(board); break;
			case 65: success = moveUp(board);    break;
			case 66: success = moveDown(board);  break;
			default: success = false;
		}
		if (success) {
			drawBoard(board);
			usleep(150000);
			addRandom(board);
			drawBoard(board);
			if (gameEnded(board)) break;
		}
	} while (c!='q');
	setBufferedInput(true);

	printf("GAME OVER\n");

	return EXIT_SUCCESS;
}

