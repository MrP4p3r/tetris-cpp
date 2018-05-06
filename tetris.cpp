/*
 * coding: utf-8
 *
 * Tetris
 *
 * GG, 2013
 */

#include <stdio.h>
#include <stdlib.h>

#include <ctime>
#include <time.h>
#include <string>

#include <GL/glut.h>
// #include <audiere.h>

#define BLOCK_STATIC 'S'
#define BLOCK_DYNAMIC 'D'
#define BLOCK_EMPTY 'E'

#define TETR_I 0
#define TETR_J 1
#define TETR_L 2
#define TETR_O 3
#define TETR_S 4
#define TETR_T 5
#define TETR_Z 6

#define DIR_CW true
#define DIR_ACW false
#define DIR_LEFT 0
#define DIR_RIGHT 1

#define CLOCKS_PER_SECOND CLOCKS_PER_SEC/10
//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

//using namespace audiere;

struct fpoints
{
	GLfloat top;
	GLfloat bot;
	GLfloat left;
	GLfloat right;
	GLfloat mid_x;
	GLfloat mid_y;
};

struct point
{
	GLfloat x;
	GLfloat y;
};

struct intpoint
{
	int x;
	int y;
};

struct color
{
	GLfloat r, g, b;
};

class block
{
public:
	color col;  //цвет блока
	char type;	//тип блока (статический S, динамический D, пустой E)
	void draw(int x, int y);	//рисует блок
};

class game
{
public:
	block **blocks; //массив с блоками
	bool lossFlag; //флаг проигрыша
	int points; //просто очки
	char *points_str; //строка с очками
	int block_size;	//размер блока
	
	class tetr
	{
	public:
		bool felt;
		char type;
		char type_new;
		color col;
		color col_new;
		
		intpoint tetr_block[4];
		intpoint tetr_block_new[4];
		void newTetr();
		void fillColors();
		color colors[7];
	private:		
		void init(intpoint *tetr_block, char itype);
	} TETR;

	void initGame();
	void speedUp(bool a);

	void translate(bool dir);
	void rotate(bool dir);
	bool moveDown();
	void lockTetr();

	void drawField();
	void drawTetrs();
	void processField();
private:
	double timer;
	double delay;
	fpoints field;
	void initBlocks();
	bool removeLines();
	void showTetr();
	void hideTetr();
	void inttostr(int a, char *str);
} GAME ;

int res_x=1280;
int res_y=720;

GLfloat max_x=res_x;
GLfloat max_y=res_y;

int timer_delay=1;

bool KEYS[256];
// AudioDevicePtr device(OpenDevice());
// OutputStreamPtr sound(OpenSound(device,"tetris.mp3", false));

void disp();
void kbdw(unsigned char KEY, int X, int Y);
void kbup(unsigned char KEY, int X, int Y);
void kbActions();
void timr(int value);
void printString(const char *text, int x, int y);
void printValue(const int val, int x, int y);

int main(int argcp, char **argv)
{
	setlocale(LC_ALL,"Russian");
	srand(unsigned(time(0)));
	glutInit(&argcp,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(00, 00);
	glutInitWindowSize(res_x, res_y);
	glutCreateWindow("TETRIS");
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, max_x, 0.0, max_y, -1.0, 1.0);
	glutDisplayFunc(&disp);
	glutKeyboardFunc(&kbdw);
	glutKeyboardUpFunc(&kbup);
	GAME.initGame();
	glutTimerFunc(timer_delay,&timr,0);

	glutMainLoop();
}

void disp()
{
	kbActions();
	GAME.processField();
	glClear(GL_COLOR_BUFFER_BIT);
	GAME.drawTetrs();
	GAME.drawField();
	glutSwapBuffers();
}

void game::initGame()
{
	field.top = max_y*0.975;
	field.left = max_x/2-max_y/2*0.95;
	field.bot = max_y*0.025;
	field.right = max_x-field.left;
	block_size = int(max_y*0.95/20)-1;
	field.mid_x = field.left + 10 * (block_size + 1);
	field.mid_y = field.bot + 10 * (block_size + 1);
	initBlocks();
	TETR.fillColors();
	timer = std::clock();
	delay = 0.8*CLOCKS_PER_SECOND; //задержка = 0.8s
	char type = rand() % 7;
	TETR.col_new = TETR.colors[type];
	TETR.type_new = type;
	TETR.newTetr();
	TETR.felt = false;
	lossFlag = false;
	points = 0;
	if (points_str) delete(points_str);
	points_str = new char[7];
	for (int i = 0; i < 7; i++) points_str[i] = '0';
	// sound->play();
	// sound->reset();
	// sound->setRepeat(true);
	// sound->setVolume(0.25f);
}

void game::drawField()
{
	GLfloat X,Y,A;
	//Рисую основную рамку
	glColor3f(1.0,GLfloat(!lossFlag),GLfloat(!lossFlag));
	glBegin(GL_LINE_LOOP);
		glVertex2f(field.left-1 , field.top+2);
		glVertex2f(field.left-1 , field.bot);
		glVertex2f(field.right+1, field.bot);
		glVertex2f(field.right+1, field.top+2);
	glEnd();
	glBegin(GL_LINES);
		glVertex2f(field.mid_x+1, field.top+2);
		glVertex2f(field.mid_x+1, field.bot);
	glEnd();
	//Рисую решетку
	glColor3f(0.05,0.05,0.05);
	glBegin(GL_LINES);
	for (int y = 1; y < 21; y++)
	{
		Y=field.bot+(y)*(block_size+1)+1;
		glVertex2f(field.left,Y);
		glVertex2f(field.mid_x,Y);
	}
	for (int x = 1; x < 11; x++)
	{
		X=field.left+(x)*(block_size+1);
		glVertex2f(X, field.bot);
		glVertex2f(X, field.top);
	}
	glEnd();

	//Рисую рамку для новой фигуры
	glColor3f(1.0,GLfloat(!lossFlag),GLfloat(!lossFlag));
	X=field.mid_x+3*(block_size+1)+1;
	Y=field.bot+15*(block_size+1);
	A=block_size+1;
	glBegin(GL_LINE_LOOP);
		glVertex2f(X-1    , Y+1);
		glVertex2f(X-1    , Y-4*A-1);
		glVertex2f(X+4*A+1, Y-4*A-1);
		glVertex2f(X+4*A+1, Y+1);
	glEnd();

	printString("NEXT:", int(X), int (Y+5));
	printString("SCORE:", int(X), int(Y-4*A-30));
	printString(points_str, int(X), int(Y-4*A-42));
	printString("CONTROLS:", int(X), int(Y-4*A-66));
	printString("A/D---LEFT/RIGHT", int(X), int(Y-4*A-78));
	printString("W-----ROTATION", int(X), int(Y-4*A-90));
	printString("S-----SPEED UP", int(X), int(Y-4*A-102));
	printString("R-----RESTART", int(X), int(Y-4*A-114));
	printString("Esc---EXIT", int(X), int(Y-4*A-126));

	//Рисую решетку для новой фигуры. Для красоты
	glColor3f(0.05,0.05,0.05);
	glBegin(GL_LINES);
	for (int y = 1; y < 4; y++)
	{
		glVertex2f(X    , Y-A*y);
		glVertex2f(X+4*A, Y-A*y);
	}
	for (int x = 1; x < 4; x++)
	{
		glVertex2f(X+A*x, Y);
		glVertex2f(X+A*x, Y-A*4);
	}
	glEnd();

	//Рисую следующую фигуру
	glColor3f(TETR.col_new.r, TETR.col_new.g, TETR.col_new.b);
	X+=2*(block_size+1)+1;
	Y-=2*(block_size+1)+1;
	glBegin(GL_QUADS);
		glVertex2f(X,Y);
		glVertex2f(X,Y-(block_size));
		glVertex2f(X+(block_size),Y-(block_size));
		glVertex2f(X+(block_size),Y);
	for ( int i = 1; i < 4; i++)
	{
		glVertex2f(X+TETR.tetr_block_new[i].x*(block_size+1),Y+TETR.tetr_block_new[i].y*(block_size+1));
		glVertex2f(X+TETR.tetr_block_new[i].x*(block_size+1),Y+TETR.tetr_block_new[i].y*(block_size+1)-(block_size));
		glVertex2f(X+TETR.tetr_block_new[i].x*(block_size+1)+(block_size),Y+TETR.tetr_block_new[i].y*(block_size+1)-(block_size));
		glVertex2f(X+TETR.tetr_block_new[i].x*(block_size+1)+(block_size),Y+TETR.tetr_block_new[i].y*(block_size+1));
	}
	glEnd();
}

void game::drawTetrs()
{
	GLfloat X,Y;
	for (int y = 1; y < 21; y++)
	{
		for (int x=1; x<11; x++)
		{
			X=field.left+(x-1)*(block_size+1);
			Y=field.bot+y*(block_size+1);
			glColor3f(blocks[y][x].col.r, blocks[y][x].col.g, blocks[y][x].col.b);
			glBegin(GL_QUADS);
				glVertex2f(X,Y);
				glVertex2f(X,Y-block_size);
				glVertex2f(X+block_size,Y-block_size);
				glVertex2f(X+block_size,Y);
			glEnd();
		}
	}


}

void game::initBlocks()
{
	if(!blocks)
	{
		blocks = new block*[25];
		//0 - дно, 1..20 - основное поле, 21..24 - появления фигур
		for (int i = 0; i < 25; i++)
		{
			blocks[i] = new block[12]; //10 столбцов
		}
	}
	for(int i = 0; i < 25; i++)
	{
		for (int j = 0; j<12; j++)
		{
			blocks[i][j].col.r=0;
			blocks[i][j].col.g=0;
			blocks[i][j].col.b=0;
			blocks[i][j].type=BLOCK_EMPTY;
		}
	}
	for (int i = 0; i<12; i++)
	{
		blocks[0][i].type=BLOCK_STATIC;
	}
	for (int i = 1; i < 25; i++)
	{
		blocks[i][0].type=BLOCK_STATIC;
		blocks[i][11].type=BLOCK_STATIC;
	}
}

void game::processField()
{
	if (delay <= std::clock() - timer && !lossFlag) //если время пришло
	{
		timer = std::clock(); //"обнуляем" таймер
		if (TETR.felt) //если фигура упала
		{
			removeLines(); //удалем заполненные строки
			TETR.felt = false;
		}
		else //если не упала продвигаем ее вниз
		{
			hideTetr();
			moveDown();
			showTetr();
			if (TETR.felt) //если фигура упала
			{
				lockTetr(); //создаем новую фигуру
				if (TETR.tetr_block[0].y>19)
				{
					lossFlag = true;
					// sound->stop();
					// sound->reset();
				}
				TETR.newTetr();
			}
		}
		return;
	}
}

void game::hideTetr()
{
	int x,y;
	x = TETR.tetr_block[0].x;
	y = TETR.tetr_block[0].y;
	blocks[y][x].col.r = 0;
	blocks[y][x].col.g = 0;
	blocks[y][x].col.b = 0;
	blocks[y][x].type = BLOCK_EMPTY;
	for ( int i = 1; i < 4; i++)
	{
		x = TETR.tetr_block[0].x + TETR.tetr_block[i].x;
		y = TETR.tetr_block[0].y + TETR.tetr_block[i].y;
		blocks[y][x].col.r = 0;
		blocks[y][x].col.g = 0;
		blocks[y][x].col.b = 0;
		blocks[y][x].type = BLOCK_EMPTY;
	}
}

void game::showTetr()
{
	int x,y;
	x = TETR.tetr_block[0].x;
	y = TETR.tetr_block[0].y;
	blocks[y][x].col = TETR.col;
	blocks[y][x].type = BLOCK_DYNAMIC;
	for ( int i = 1; i < 4; i++)
	{
		x = TETR.tetr_block[0].x + TETR.tetr_block[i].x;
		y = TETR.tetr_block[0].y + TETR.tetr_block[i].y;
		blocks[y][x].col = TETR.col;
		blocks[y][x].type = BLOCK_DYNAMIC;
	}
}

bool game::removeLines()
{
	bool flag=false;
	bool emptflag=false;
	int i = 0;
	for ( int y = 1; y < 21 && !emptflag; y++)
	{
		emptflag=true;
		if (flag) y--;
		flag = true;
		for ( int x = 1; x < 11; x++)
		{
			if (blocks[y][x].type == BLOCK_EMPTY) flag = false;
			else emptflag=false;
		}
		if (flag && !emptflag)
		{
			for ( int qy = y; qy < 21; qy++)
			{
				for ( int qx = 1; qx < 11; qx++)
				{
					blocks[qy][qx].col = blocks[qy+1][qx].col;
					blocks[qy][qx].type = blocks[qy+1][qx].type;
				}
			}
			i = 2 * i + 100;
		}
	}
	points += i;
	inttostr(points,points_str);
	return 0; // не удалилось ничего
}

void game::tetr::init(intpoint *tetr_block, char itype)
{
	//центр фигуры:
	tetr_block[0].x=6;
	tetr_block[0].y=21;
	//остальные точки относительно центра в зависимости от type
	tetr_block[1].x=0;
	tetr_block[1].y=1;
	switch (itype)
	{
	case TETR_I:
		tetr_block[0].y+=1;
		tetr_block[2].x=0;
		tetr_block[2].y=2;
		tetr_block[3].x=0;
		tetr_block[3].y=-1;
	break;
	case TETR_J:
		tetr_block[0].y+=1;
		tetr_block[2].x=0;
		tetr_block[2].y=-1;
		tetr_block[3].x=-1;
		tetr_block[3].y=-1;
	break;
	case TETR_L:
		tetr_block[0].y+=1;
		tetr_block[2].x=0;
		tetr_block[2].y=-1;
		tetr_block[3].x=1;
		tetr_block[3].y=-1;
	break;
	case TETR_O:
		tetr_block[2].x=-1;
		tetr_block[2].y=0;
		tetr_block[3].x=-1;
		tetr_block[3].y=1;
	break;
	case TETR_S:
		tetr_block[2].x=-1;
		tetr_block[2].y=0;
		tetr_block[3].x=1;
		tetr_block[3].y=1;
	break;
	case TETR_T:
		tetr_block[2].x=1;
		tetr_block[2].y=0;
		tetr_block[3].x=-1;
		tetr_block[3].y=0;
	break;
	case TETR_Z:
		tetr_block[2].x=-1;
		tetr_block[2].y=1;
		tetr_block[3].x=1;
		tetr_block[3].y=0;
	break;
	}
}

void game::tetr::newTetr()
{
	type = type_new;
	col = col_new;
	type_new = rand() % 7;
	col_new = colors[type_new];
	init(tetr_block, type);
	init(tetr_block_new, type_new);
}

void game::tetr::fillColors()
{
	colors[TETR_I].r = 0.192;
	colors[TETR_I].g = 0.78;
	colors[TETR_I].b = 0.937;

	colors[TETR_O].r = 0.969;
	colors[TETR_O].g = 0.827;
	colors[TETR_O].b = 0.31;

	colors[TETR_L].r = 0.937;
	colors[TETR_L].g = 0.475;
	colors[TETR_L].b = 0.129;

	colors[TETR_J].r = 0.;
	colors[TETR_J].g = 0.;
	colors[TETR_J].b = 1.;

	colors[TETR_Z].r = 1.;
	colors[TETR_Z].g = 0.;
	colors[TETR_Z].b = 0.;

	colors[TETR_S].r = 0.;
	colors[TETR_S].g = 1.;
	colors[TETR_S].b = 0.;

	colors[TETR_T].r = 0.678;
	colors[TETR_T].g = 0.302;
	colors[TETR_T].b = 0.612;
}

bool game::moveDown()
{
	int x,y;
	bool flag = true;
	x = TETR.tetr_block[0].x;
	y = TETR.tetr_block[0].y-1;
	if (blocks[y][x].type == BLOCK_STATIC) flag = false;
	for ( int i = 1; i < 4; i++)
	{
		x = TETR.tetr_block[i].x + TETR.tetr_block[0].x;
		y = TETR.tetr_block[i].y + TETR.tetr_block[0].y - 1;
		if (blocks[y][x].type == BLOCK_STATIC) flag = false;
	}
	if (flag)
	{
		TETR.tetr_block[0].y-=1;
		return 0;
	}
	else
	{
		TETR.felt = true;
		return 1;
	}
}

void game::rotate(bool dir)
{
	if (TETR.type == TETR_O) return;
	int t,x,y;
	bool flag = true;
	hideTetr();
	if (dir == DIR_ACW)
	{
		for ( int i = 1; i < 4; i++)
		{
			y = TETR.tetr_block[i].x;
			x = - TETR.tetr_block[i].y;
			y += TETR.tetr_block[0].y;
			x += TETR.tetr_block[0].x;
			if (blocks[y][x].type == BLOCK_STATIC) flag = false;
		}
		if (flag)
		{
			for ( int i = 1; i < 4; i++)
			{
				t = TETR.tetr_block[i].y;
				TETR.tetr_block[i].y = TETR.tetr_block[i].x;
				TETR.tetr_block[i].x = -t;
			}
		}
	}
	else
	{
		for ( int i = 1; i < 4; i++)
		{
			y = - TETR.tetr_block[i].x;
			x = TETR.tetr_block[i].y;
			y += TETR.tetr_block[0].y;
			x += TETR.tetr_block[0].x;
			if (blocks[y][x].type == BLOCK_STATIC) flag = false;
		}
		if (flag)
		{
			for ( int i = 1; i < 4; i++)
			{
				t = TETR.tetr_block[i].x;
				TETR.tetr_block[i].x = TETR.tetr_block[i].y;
				TETR.tetr_block[i].y = -t;
			}
		}
	}
	showTetr();
}

void game::translate(bool dir)
{
	hideTetr();
	bool flag = true;
	int x,y;
	if(dir == DIR_RIGHT)
	{
		x = TETR.tetr_block[0].x + 1;
		y = TETR.tetr_block[0].y;
		if (x==11 || blocks[y][x].type==BLOCK_STATIC)
		{
			flag = false;
		}
		for (int i = 1; i < 4; i++)
		{
			x = TETR.tetr_block[0].x + TETR.tetr_block[i].x + 1;
			y = TETR.tetr_block[0].y + TETR.tetr_block[i].y;
			if (x==1 || blocks[y][x].type==BLOCK_STATIC)
			{
				flag = false;
			}
		}
		if (flag) TETR.tetr_block[0].x+=1;
	}
	else
	{
		x = TETR.tetr_block[0].x - 1;
		y = TETR.tetr_block[0].y;
		if (x==0 || blocks[y][x].type==BLOCK_STATIC)
		{
			flag = false;
		}
		for (int i = 1; i < 4; i++)
		{
			x = TETR.tetr_block[0].x + TETR.tetr_block[i].x - 1;
			y = TETR.tetr_block[0].y + TETR.tetr_block[i].y;
			if (x==0 || blocks[y][x].type==BLOCK_STATIC)
			{
				flag = false;
			}
		}
		if (flag) TETR.tetr_block[0].x-=1;
	}
	showTetr();
}

void game::lockTetr()
{
	int x,y;
	x = TETR.tetr_block[0].x;
	y = TETR.tetr_block[0].y;
	blocks[y][x].type = BLOCK_STATIC;
	for (int i = 1; i < 4; i++)
	{
		x = TETR.tetr_block[0].x + TETR.tetr_block[i].x;
		y = TETR.tetr_block[0].y + TETR.tetr_block[i].y;
		blocks[y][x].type = BLOCK_STATIC;
	}
}

void game::speedUp(bool a)
{
	if (a)
	{
		delay = 0.05 * CLOCKS_PER_SECOND;
	}
	else
	{
		delay = 0.7 * CLOCKS_PER_SECOND;
	}
}

void kbdw(unsigned char KEY, int X, int Y)
{
	KEYS[KEY]=true;
}

void kbup(unsigned char KEY, int X, int Y)
{
	KEYS[KEY]=false;
}

void kbActions()
{
	if(KEYS[27])
	{
		exit(0);
	}
	if(KEYS['a'])
	{
		GAME.translate(DIR_LEFT);
		KEYS['a'] = false;
	}
	if(KEYS['d'])
	{
		GAME.translate(DIR_RIGHT);
		KEYS['d'] = false;
	}
	if(KEYS['t'])
	{
		printf("\n\%i\n",GAME.TETR.type);
	}
	if(KEYS['w'])
	{
		GAME.rotate(DIR_CW);
		KEYS['w']=false;
	}
	if(KEYS['s']) GAME.speedUp(1);
	else GAME.speedUp(0);
	if(KEYS['r'])
	{
		GAME.initGame();
		KEYS['r'] = false;
	}
}

void timr(int value)
{
	glutPostRedisplay();
	glutTimerFunc(timer_delay,&timr,0);
}

void printString(const char *text, int x, int y)
{
	glRasterPos2f(x, y);
	while (*text) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *text++);
}

void game::inttostr(int a, char *str)
{
	int buf;
	for (int i = 6; i>=0; i--)
	{
		buf = a % 10;
		a = a / 10;
		str[i] = buf + 0x30;
	}
}

