#include "tetris.h"
#include <unistd.h>
#include <malloc.h>
#include <time.h>

static struct sigaction act, oact;

int main(){
	int exit=0;

	createRankList();
	initscr();
	noecho();
	keypad(stdscr, TRUE);	

	srand((unsigned int)time(NULL));

	while(!exit){
		clear();
		reco = 0;
		switch(menu()){
		case MENU_PLAY: play(); break;
		case MENU_RANK: rank(); break;
		case MENU_RECO: recommendedPlay(); break;
		case MENU_EXIT: exit=1; break;
		default: break;
		}
	}

	endwin();
	writeRankFile();
	system("clear");
	return 0;
}

void InitTetris(){
	int i,j;

	for(j=0;j<HEIGHT;j++)
		for(i=0;i<WIDTH;i++)
			field[j][i]=0;

	nextBlock[0]=rand()%7;
	nextBlock[1]=rand()%7;
	nextBlock[2] = rand() % 7;
	blockRotate=0;
	blockY=-1;
	blockX=WIDTH/2-2;
	score=0;	
	gameOver=0;
	timed_out=0;

	DrawOutline();
	DrawField();
	recRoot = NULL;
	DrawBlockWithFeatures(blockY,blockX,nextBlock[0],blockRotate);
	DrawNextBlock(nextBlock);
	PrintScore();
}

void DrawOutline(){	
	int i,j;
	/* 블럭이 떨어지는 공간의 태두리를 그린다.*/
	DrawBox(0,0,HEIGHT,WIDTH);

	/* next block을 보여주는 공간의 태두리를 그린다.*/
	move(2, WIDTH + 10);
	printw("NEXT BLOCK");
	DrawBox(3,WIDTH+10,4,8);
	move(2, WIDTH + 10);
	DrawBox(9, WIDTH + 10, 4, 8);

	/* score를 보여주는 공간의 태두리를 그린다.*/
	move(15,WIDTH+10);
	printw("SCORE");
	DrawBox(16,WIDTH+10,1,8);
}

int GetCommand(){
	int command;
	command = wgetch(stdscr);
	switch(command){
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case ' ':	/* space key*/
		/*fall block*/
		break;
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

int ProcessCommand(int command){
	int ret=1;
	int drawFlag=0;
	switch(command){
	case QUIT:
		ret = QUIT;
		break;
	case KEY_UP:
		if((drawFlag = CheckToMove(field,nextBlock[0],(blockRotate+1)%4,blockY,blockX)))
			blockRotate=(blockRotate+1)%4;
		break;
	case KEY_DOWN:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX)))
			blockY++;
		break;
	case KEY_RIGHT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX+1)))
			blockX++;
		break;
	case KEY_LEFT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX-1)))
			blockX--;
		break;
	default:
		break;
	}
	if(drawFlag) DrawChange(field,command,nextBlock[0],blockRotate,blockY,blockX);
	return ret;	
}

void DrawField(){
	int i,j;
	for(j=0;j<HEIGHT;j++){
		move(j+1,1);
		for(i=0;i<WIDTH;i++){
			if(field[j][i]==1){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(".");
		}
	}
}


void PrintScore(){
	move(17,WIDTH+11);
	printw("%8d", score);
}

void DrawNextBlock(int *nextBlock){
	int i, j;
	for( i = 0; i < 4; i++ ){
		move(4+i,WIDTH+13);
		for( j = 0; j < 4; j++ ){
			if( block[nextBlock[1]][0][i][j] == 1 ){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}
	for (i = 0; i < 4; i++) {
		move(10 + i, WIDTH + 13);
		for (j = 0; j < 4; j++) {
			if (block[nextBlock[2]][0][i][j] == 1) {
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}
}

void DrawBlockWithFeatures(int y, int x, int blockID, int blockRotate) {
	if(reco != 0) DrawRecommend(blockY, blockX, nextBlock[0], blockRotate);
	if(reco == 0) DrawShadow(y, x, blockID, blockRotate);
	DrawBlock(blockY, blockX, nextBlock[0], blockRotate, ' ');

}


void DrawBlock(int y, int x, int blockID,int blockRotate,char tile){
	int i,j;
	

	for(i=0;i<4;i++)
		for(j=0;j<4;j++){
			if(block[blockID][blockRotate][i][j]==1 && i+y>=0){
				move(i+y+1,j+x+1);
				attron(A_REVERSE);
				printw("%c",tile);
				attroff(A_REVERSE);
			}
		}

	move(HEIGHT,WIDTH+10);
}

void DrawBox(int y,int x, int height, int width){
	int i,j;
	move(y,x);
	addch(ACS_ULCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_URCORNER);
	for(j=0;j<height;j++){
		move(y+j+1,x);
		addch(ACS_VLINE);
		move(y+j+1,x+width+1);
		addch(ACS_VLINE);
	}
	move(y+j+1,x);
	addch(ACS_LLCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_LRCORNER);
}

void play(){
	int command;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM, &act, &oact);
	InitTetris();
	do {
		if (timed_out == 0) {
			alarm(1);
			timed_out = 1;
		}

		command = GetCommand();
		if (ProcessCommand(command) == QUIT) {
			alarm(0);
			DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
			move(HEIGHT / 2, WIDTH / 2 - 4);
			printw("Good-bye!!");
			refresh();
			getch();

			return;
		}
	} while (!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
	move(HEIGHT / 2, WIDTH / 2 - 4);
	printw("GameOver!!");
	refresh();
	newRank(score);
	getch();
}

char menu(){
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");
	return wgetch(stdscr);
}

/////////////////////////첫주차 실습에서 구현해야 할 함수/////////////////////////

int CheckToMove(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
	int flag = 1;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (block[currentBlock][blockRotate][i][j] == 1) {
				if ((blockX + j) < 0 || (blockX + j) >= WIDTH) {
					return 0;
				}
				if ((blockY + i) < 0 || (blockY + i) >= HEIGHT) {
					return 0;
				}
				if (f[blockY + i][blockX + j] == 1) return 0;
			}
		}
	}

	return 1;
}

void DrawChange(char f[HEIGHT][WIDTH],int command,int currentBlock,int blockRotate, int blockY, int blockX){
	// user code

	//1. 이전 블록 정보를 찾는다. ProcessCommand의 switch문을 참조할 것
	//2. 이전 블록 정보를 지운다. DrawBlock함수 참조할 것.
	//3. 새로운 블록 정보를 그린다. 

	int r = blockRotate;
	int x = blockX;
	int y = blockY;

	switch (command) {
	case KEY_UP:
		r = (blockRotate + 3) % 4;
		break;
	case KEY_DOWN:
		y = blockY-1;
		break;
	case KEY_RIGHT:
		x = blockX-1;
		break;
	case KEY_LEFT:
		x = blockX+1;
		break;
	default:
		break;
	}
	int shadow = y;
	while (CheckToMove(field, nextBlock[0], r, shadow + 1, x)) {
		shadow++;
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (block[currentBlock][r][i][j] == 1 && i + shadow >= 0) {
				move(i + shadow + 1, j + x + 1);
				printw("%c", '.');
			}
		}
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (block[currentBlock][r][i][j] == 1 && i + y >= 0) {
				move(i + y + 1, j + x + 1);
				printw("%c", '.');
			}
		}
	}

	DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate);
	move(HEIGHT, WIDTH + 10);
}

void BlockDown(int sig){
	// user code

	//강의자료 p26-27의 플로우차트를 참고한다.
	if (!CheckToMove(field, nextBlock[0], blockRotate, blockY + 1, blockX)) {
		if (blockY == -1) gameOver = 1;
		score += 10 * AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX);
		score += DeleteLine(field);
		nextBlock[0] = nextBlock[1];
		nextBlock[1] = nextBlock[2];
		nextBlock[2] = rand() % 7;
		blockRotate = 0;
		blockY = -1;
		blockX = WIDTH / 2 - 2;
		DrawNextBlock(nextBlock);
		PrintScore();
		DrawField();
		recRoot = NULL; // 채우기
	}
	else {
		if (reco == 1) {
			DrawChange(field, KEY_DOWN, nextBlock[0], blockRotate, blockY, blockX);
			blockRotate = recRoot->recBlockRotate; blockY = recRoot->recBlockY; blockX = recRoot->recBlockX;
		}
		else {
			blockY++;
			DrawChange(field, KEY_DOWN, nextBlock[0], blockRotate, blockY, blockX);

		}

	}
	g_time++;
	timed_out = 0;
}

int AddBlockToField(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
	int touched = 0;

	//Block이 추가된 영역의 필드값을 바꾼다.
	for (int i = 0; i<4; i++) {
		for (int j = 0; j<4; j++) {
			if (block[currentBlock][blockRotate][i][j]==1) {
				if (blockY + i + 1 >= HEIGHT) touched++;
				else if (f[blockY + i + 1][blockX + j] == 1) touched++;
				f[blockY+i][blockX+j] = 1;
			}
		}
	}
	return touched;
}

int DeleteLine(char f[HEIGHT][WIDTH]){
	// user code

	//1. 필드를 탐색하여, 꽉 찬 구간이 있는지 탐색한다.
	//2. 꽉 찬 구간이 있으면 해당 구간을 지운다. 즉, 해당 구간으로 필드값을 한칸씩 내린다.
	int line = HEIGHT-1;
	int flag;
	int ret = 0;
	while (line>=0) {
		flag = 1;
		for (int i = 0; i < WIDTH; i++) {
			if (f[line][i] == 0) {
				flag = 0;
				break;
			}
		}
		if (flag) {
			for (int i = line; i > 0; i--) {
				for (int j = 0; j < WIDTH; j++) {
					f[i][j] = f[i - 1][j];
				}
			}
			for (int j = 0; j < WIDTH; j++) f[0][j] = 0;
			ret++;
		}
		else line--;


	}
	return ret*ret * 100;
}

///////////////////////////////////////////////////////////////////////////

void DrawShadow(int y, int x, int blockID,int blockRotate){
	// user code
	int shadow = y;
	while (CheckToMove(field, nextBlock[0], blockRotate, shadow + 1, x)) {
		shadow++;
	}

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
			if (block[blockID][blockRotate][i][j] == 1 && i + shadow >= 0) {
				move(i + shadow + 1, j + x + 1);
				attron(A_REVERSE);
				printw("%c", '/');
				attroff(A_REVERSE);
			}
		}

	move(HEIGHT, WIDTH + 10);
}

void createRankList() {
	// user code
	FILE* file = fopen("rank.txt", "r");
	if (file == NULL) return;
	fscanf(file, "%d", &ranknum);
	for (int i = 0; i < ranknum; i++) {
		char nm[NAMELEN];
		int sc;
		fscanf(file, "%s %d", nm, &sc);
		int idx = binary(i, nm, sc);

		for (int j = i; j > idx; j--) {
			rankarr[j] = rankarr[j - 1];
		}

		rankarr[idx].score = sc; strcpy(rankarr[idx].name, nm);
	}
}

int binary(int n, char name[], int sc) {
	int left = 0;
	int right = n;
	while (left < right) {
		int mid = (left + right) / 2;
		if (rankarr[mid].score == sc) {
			return mid + 1;
		}
		else if (rankarr[mid].score < sc) {
			right = mid;
		}
		else left = mid + 1;
	}
	return left;
}

void rank() {

	int X = 1, Y = ranknum;
	char buf[100];
	int flag = 0;
	int idx;
	buf[0] = '\n';
	// user code
	initscr();
	noecho();
	keypad(stdscr, TRUE);

	while (1) 
	{
		clear();
		noecho();
		printw("1. list ranks from X to Y\n");
		printw("2. list ranks by a specific name\n");
		printw("3. delete\n");
		printw("4. exit\n");
		switch (wgetch(stdscr)) {
		case '1':
			X = 1; Y = ranknum;
			echo();

			printw("X :"); 
			scanw("%d", &X);
			printw("Y :"); 
			scanw("%d", &Y);

			if (Y > ranknum || X <= 0 || Y <= 0 || X>Y) {
				initscr();
				noecho();
				keypad(stdscr, TRUE);	
				break;
			}

			for (int i = X - 1; i < Y; i++) {
				printw("%s %d\n", rankarr[i].name, rankarr[i].score);
			}
			scanw("%s", buf);
			break;
		case '2':
			flag = 0;
			echo();
			printw("input name : ");
			scanw("%s", buf);
			for (int i = 0; i < ranknum; i++) {
				if (strcmp(rankarr[i].name, buf) == 0) {
					printw("%s %d\n", buf, rankarr[i].score);
					flag = 1;
				}
			}
			if (flag == 0) printw("search failure: no name in the list\n");

			scanw("%s", buf);
			noecho();
			break;
		case '3':
			flag = 0;
			echo();
			printw("input delele rank : ");
			scanw("%d", &idx);
			if (ranknum < idx) {
				printw("search failure: the rank not in the list\n");
			}
			else {
				printw("result: the rank deleted\n");
				for (int i = idx-1; i < ranknum-1; i++) {
					rankarr[i] = rankarr[i + 1];
				}
				ranknum--;
			}

			scanw("%s", buf);
			break;
		case '4':
			clear();
			initscr();
			noecho();
			keypad(stdscr, TRUE);
			return;

		}
	}
}

void writeRankFile() {
	// user code
	FILE* file = fopen("rank.txt", "w");

	fprintf(file, "%d\n", ranknum);
	for (int i = 0; i < ranknum; i++) {
		fprintf(file, "%s %d\n", rankarr[i].name, rankarr[i].score);
	}
	fclose(file);
}

void newRank(int score) {
	// user code

	clear();
	refresh();
	endwin();


	char nm[NAMELEN];
	printf("input your name : ");
	scanf("%s", nm);
	int idx = binary(ranknum, nm, score);

	for (int j = ranknum; j > idx; j--) {
		rankarr[j] = rankarr[j - 1];
	}
	rankarr[idx].score = score; strcpy(rankarr[idx].name, nm);

	ranknum++;

	initscr();
	noecho();
	keypad(stdscr, TRUE);
}

long evalSize(RecNode* Sroot) {
	long sum = sizeof(*Sroot);
	if (Sroot->child == 0) return sizeof(*Sroot);
	for (int i = 0; i < Sroot->child; i++) {
		sum += evalSize(&(Sroot->c[i]));
	}
	return sum;

}

void DrawRecommend(int y, int x, int blockID, int blockRotate) {
	// user code
	//recRoot 처음에 1st에서 n개 만큼 반복하고 (blockX + j) >= WIDTH) 이거 관련 어쩌구


	//typedef struct _RecNode {
	//	int lv, score;
	//	char f[HEIGHT][WIDTH];
	//	int accumulatedScore;
	//	struct _RecNode* c[CHILDREN_MAX];
	//	int curBlockID;
	//	int recBlockX, recBlockY, recBlockRotate;
	//	struct _RecNode* parent;

	//} RecNode;

	//field는 22 * 10
	if(!recRoot) modified_recommend(&recRoot);
	DrawBlock(recRoot->recBlockY, recRoot->recBlockX, recRoot->curBlockID, recRoot->recBlockRotate, 'R');

	move(19, WIDTH + 10);
	printw("game time : %d    ", g_time);
	move(20, WIDTH + 10);
	printw("score/t : %.5f    ", (double)score / g_time);
	move(21, WIDTH + 10);
	printw("score(t)/time(t) : %.1f   ", (double)(score / duration));
	move(22, WIDTH + 10);
	printw("score(t)/space(t) : %.10f    ", (double)score / size);
}

void freeNode(RecNode* node) {
	if (node->child == 0) return;
	for (int i = 0; i < node->child; i++) {
		freeNode(&(node->c[i]));
	}
	free(node->c);
}


int modified_recommend(RecNode** root) {
	time_t start, stop;
	start = time(NULL);
	//clock_t start, stop;
	//start = clock();
	int max = -1000000; // 미리 보이는 블럭의 추천 배치까지 고려했을 때 얻을 수 있는 최대 점수
	RecNode zero;
	zero.lv = 0; zero.score = 0; zero.parent = NULL; zero.accumulatedScore = 0; zero.child = 0;
	for (int i = 0; i < HEIGHT; i++)
		for (int j = 0; j < WIDTH; j++) zero.f[i][j] = field[i][j];
	//nextBlock[]

	RecNode* cur[100000];
	RecNode* next[100000];
	int curN = 1, nextN = 0;

	cur[0] = &zero;
	RecNode* best3 = NULL;
	RecNode* node = &zero;

	for (int idx = 0; idx < VISIBLE_BLOCKS; idx++) {
		int ch = nextBlock[idx];
		nextN = 0;

		for (int i = 0; i < curN; i++) {
			int n = 0;
			for (int r = 0; r < ro[ch]; r++) n += right[ch][r] - left[ch][r] + 1;

			cur[i]->c = (RecNode*)malloc(n * sizeof(RecNode));
			cur[i]->child = n;

			int num = 0;

			for (int r = 0; r < ro[ch]; r++) {
				for (int x = left[ch][r]; x <= right[ch][r]; x++) {
					RecNode* node = &(cur[i]->c[num]);
					node->lv = idx + 1; node->curBlockID = ch; node->recBlockX = x; node->recBlockRotate = r; node->child = 0; node->recBlockY = 0;

					node->parent = cur[i];

					for (int h = 0; h < HEIGHT; h++) {
						for (int w = 0; w < WIDTH; w++) {
							node->f[h][w] = cur[i]->f[h][w];
						}
					}

					while (CheckToMove(node->f, ch, r, node->recBlockY + 1, x)) node->recBlockY += 1;

					node->score = 0;
					node->score -= 100 * (sita[ch][r] - AddBlockToField(node->f, ch, r, node->recBlockY, x));
					node->score -= 100 * (HEIGHT - node->recBlockY);
					node->score += 3 * DeleteLine(node->f);
					node->accumulatedScore = cur[i]->accumulatedScore + node->score;

					if (idx + 1 == VISIBLE_BLOCKS && max < node->accumulatedScore) {
						max = node->accumulatedScore;
						best3 = node;
					}

					if (idx + 1 < VISIBLE_BLOCKS) {
						next[nextN++] = node;
					}
					num++;
				}

			}

		}

		for (int i = 0; i < nextN; i++) cur[i] = next[i];

		curN = nextN;

	}

	//best3를 찾고 best3->parent->parent를 root로
	*root = best3;
	while ((*root)->lv != 1) *root = (*root)->parent;
	size += (long)evalSize((*root)->parent);


	//free 해줘야할 듯
	freeNode(&zero);
	// user code
	//stop = clock();
	stop = time(NULL);
	duration += (double)(stop - start);
	
	return max;



}

int recommend(RecNode** root) {
	int max = 0; // 미리 보이는 블럭의 추천 배치까지 고려했을 때 얻을 수 있는 최대 점수
	int left[7][4] = { {0, -1, 0, -1}, {-1, -2, -1, -1}, { -1, -2, -1, -1 }, {0, 0, 0, -1}, {-1,-1,-1,-1}, { -1,-1,-1,-1 }, { -1,-1,-1,-1 } };
	int right[7][4] = { { 6, 8, 6, 8 }, { 6,6,6,7 }, { 6,6,6,7 }, { 7,8,7,7 }, { 7,7,7,7 }, { 6,7,6,7 }, { 6,7,6,7 } };
	RecNode zero;
	zero.lv = 0; zero.score = 0; zero.parent = NULL; zero.accumulatedScore = 0;
	for (int i = 0; i < HEIGHT; i++)
		for (int j = 0; j < WIDTH; j++) zero.f[i][j] = field[i][j];
	//nextBlock[]
	//아 무슨 느낌인진 알겠는데 흠... 아아아아아아ㅏ아아아아ㅏ

	int n = 0;
	for (int i = 0; i < 4; i++) n += right[nextBlock[0]][i] - left[nextBlock[0]][i] + 1;
	zero.c = (RecNode*)malloc(n * sizeof(RecNode));
	zero.child = n;
	RecNode* best3;
	RecNode* node = &zero;

	int idx = 0;
	for (int j = 0; j < 4; j++) {
		for (int k = left[nextBlock[0]][j]; k <= right[nextBlock[0]][j]; k++) {
			zero.c[idx].lv = 1;
			zero.c[idx].curBlockID = nextBlock[0];
			zero.c[idx].recBlockX = k;
			zero.c[idx].recBlockRotate = j;
			zero.c[idx].parent = &zero;

			zero.c[idx].recBlockY = 0;

			for (int h = 0; h < HEIGHT; h++) {
				for (int w = 0; w < WIDTH; w++) {
					zero.c[idx].f[h][w] = field[h][w];
				}
			}
			while (CheckToMove(zero.c[idx].f, nextBlock[0], j, zero.c[idx].recBlockY + 1, k)) {
				zero.c[idx].recBlockY += 1;
			}

			zero.c[idx].score = 0;
			zero.c[idx].score += 10 * AddBlockToField(zero.c[idx].f, nextBlock[0], j, zero.c[idx].recBlockY, k);
			zero.c[idx].score += DeleteLine(zero.c[idx].f);

			zero.c[idx].accumulatedScore = zero.c[idx].score;
			idx++;
		}

	}
	int n2 = 0;
	for (int i = 0; i < 4; i++) n2 += right[nextBlock[1]][i] - left[nextBlock[1]][i] + 1;

	for (int i = 0; i < n; i++) {
		zero.c[i].c = (RecNode*)malloc(n2 * sizeof(RecNode));
	}


	for (int i = 0; i < n; i++) {
		idx = 0;
		for (int j = 0; j < 4; j++) {
			for (int k = left[nextBlock[1]][j]; k <= right[nextBlock[1]][j]; k++) {
				zero.c[i].c[idx].lv = 2;
				zero.c[i].c[idx].curBlockID = nextBlock[1];
				zero.c[i].c[idx].recBlockX = k;
				zero.c[i].c[idx].recBlockRotate = j;
				zero.c[i].c[idx].parent = &(zero.c[i]);

				zero.c[i].c[idx].recBlockY = 0;

				for (int h = 0; h < HEIGHT; h++) {
					for (int w = 0; w < WIDTH; w++) {
						zero.c[i].c[idx].f[h][w] = zero.c[i].f[h][w];
					}
				}
				while (CheckToMove(zero.c[i].c[idx].f, nextBlock[1], j, zero.c[i].c[idx].recBlockY + 1, k)) {
					zero.c[i].c[idx].recBlockY += 1;
				}

				zero.c[i].c[idx].score = 0;
				zero.c[i].c[idx].score += 10 * AddBlockToField(zero.c[i].c[idx].f, nextBlock[1], j, zero.c[i].c[idx].recBlockY, k);
				zero.c[i].c[idx].score += DeleteLine(zero.c[i].c[idx].f);

				zero.c[i].c[idx].accumulatedScore = zero.c[i].c[idx].score + zero.c[i].accumulatedScore;

				idx++;
			}

		}
	}


	int n3 = 0;
	for (int i = 0; i < 4; i++) n3 += right[nextBlock[2]][i] - left[nextBlock[2]][i] + 1;

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n2; j++) {
			zero.c[i].c[j].c = (RecNode*)malloc(n3 * sizeof(RecNode));
		}
	}



	for (int h = 0; h < n; h++) {
		for (int i = 0; i < n2; i++) {
			idx = 0;
			for (int j = 0; j < 4; j++) {
				for (int k = left[nextBlock[2]][j]; k <= right[nextBlock[2]][j]; k++) {
					zero.c[h].c[i].c[idx].lv = 3;
					zero.c[h].c[i].c[idx].curBlockID = nextBlock[2];
					zero.c[h].c[i].c[idx].recBlockX = k;
					zero.c[h].c[i].c[idx].recBlockRotate = j;
					zero.c[h].c[i].c[idx].parent = &(zero.c[h].c[i]);

					zero.c[h].c[i].c[idx].recBlockY = 0;

					for (int h = 0; h < HEIGHT; h++) {
						for (int w = 0; w < WIDTH; w++) {
							zero.c[h].c[i].c[idx].f[h][w] = zero.c[h].c[i].f[h][w];
						}
					}
					while (CheckToMove(zero.c[h].c[i].c[idx].f, nextBlock[2], j, zero.c[h].c[i].c[idx].recBlockY + 1, k)) {
						zero.c[h].c[i].c[idx].recBlockY += 1;
					}

					zero.c[h].c[i].c[idx].score = 0;
					zero.c[h].c[i].c[idx].score += 10 * AddBlockToField(zero.c[h].c[i].c[idx].f, nextBlock[2], j, zero.c[h].c[i].c[idx].recBlockY, k);
					zero.c[h].c[i].c[idx].score += DeleteLine(zero.c[h].c[i].c[idx].f);

					zero.c[h].c[i].c[idx].accumulatedScore = zero.c[h].c[i].c[idx].score + zero.c[h].c[i].accumulatedScore;
					idx++;
				}

			}
		}
	}


	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n2; j++) {
			for (int k = 0; k < n3; k++) {
				if (max < zero.c[i].c[j].c[k].accumulatedScore) {
					best3 = &(zero.c[i].c[j].c[k]);
					max = best3->accumulatedScore;
				}
			}
		}
	}

	//best3를 찾고 best3->parent->parent를 root로
	*root = best3->parent->parent;


	//free 해줘야할 듯
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n2; j++) {
			free(zero.c[i].c[j].c);

		}
		free(zero.c[i].c);
	}
	free(zero.c);
	// user code
	return max;
}



void recommendedPlay() {
	// user code
	g_time = 1;
	size = 0;
	int command;
	int ret = 1;
	reco = 1;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM, &act, &oact);
	InitTetris();
	do {
		if (timed_out == 0) {
			alarm(1);
			timed_out = 1;
		}

		command = GetCommand();
		switch (command) {
		case QUIT:
			ret = QUIT;
			break;
		}
			if (ret == QUIT) {
				alarm(0);
				DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
				move(HEIGHT / 2, WIDTH / 2 - 4);
				printw("Good-bye!!");
				refresh();
				getch();

				return;
			}
		} while (!gameOver);

		alarm(0);
		getch();
		DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
		move(HEIGHT / 2, WIDTH / 2 - 4);
		printw("GameOver!!");
		refresh();
		getch();
}