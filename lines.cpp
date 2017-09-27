#include <cstdlib>
#include <unistd.h>
#include <iostream>

#define print(s) std::cout << s
#define TS 100000

enum colors {
	empty,
	red,
	yellow,
	green,
	cyan,
	blue,
	purple,
	brown,
	edge=-1
};

int field[11][11];

typedef struct {
	int sx, sy, dx, dy, val;
} move;

move moves[4096];
int cm = 0;

void printfield()
{
	print("\033[H ");
	for(int j = 1; j <=9 ; j++)
	{
		print(" \033[40;37;0m  " << (char)('@'+j) << "  \033[0m ");
	}
	print("\n");
	for(int i =	1; i <= 9; i++)
	{
		print(" ");
		for(int j = 1; j <= 9; j++){
			int c = field[i][j];
			print(" \033[0;4"<<c<<";3"<<c << "m   " << "  \033[0m ");
		}
		print("\n");
		print(i);
		for(int j = 1; j <= 9; j++){
			int c = field[i][j];
			if( c == 0 ){
				std::cout << " \033[0;40;30" << "m   " << "  \033[0m ";
			} else {
				std::cout << " \033[1;4"<<c<<";3" << c << "m  " << c << "  \033[0m ";
			}
		}
		print("\n ");
		for(int j = 1; j <= 9; j++){
			int c = field[i][j];
			std::cout << " \033[0;4"<<c<<";3"<<c << "m   " << "  \033[0m ";
		}
		print("\n\n");
	}

	print("\n");
}



void gameover(){
	print("Game Over");
}

bool addballs(int n=3){
	int s=0;
	for( int i = 0; i < n; s++)
	{
		int c = 1 + rand() % 7;
		int x = 1 + rand() % 9;
		int y = 1 + rand() % 9;
		
		if( field[x][y] == empty )
		{
			field[x][y] = c;
			i++;
		}
		if( s >= 10000 ) 
		{
			gameover();
			return false;
		}
	}
	return true;
}

bool movable(int x, int y){
	return 
		field[x][y]   != empty && (
		field[x+1][y] == empty ||
		field[x-1][y] == empty ||
		field[x][y-1] == empty ||
		field[x][y+1] == empty    );
}

void setupfield()
{
	for(int i=0; i < 11; i++){
		field[  0 ][  i ] = edge;
		field[ 10 ][  i ] = edge;
		field[  i ][  0 ] = edge;
		field[  i ][ 10 ] = edge;
	}
}

int localfit(int x, int y, int c){
	int g[] = {
		field[x-1][y-1] == c ? 1 : 0,
		field[ x ][y-1] == c ? 1 : 0,
		field[x+1][y-1] == c ? 1 : 0,

		field[x-1][ y ] == c ? 1 : 0,
		field[ x ][ y ] == empty ? 1 : 0,
		field[x+1][ y ] == c ? 1 : 0,
		
		field[x-1][y+1] == c ? 1 : 0,
		field[ x ][y+1] == c ? 1 : 0,
		field[x+1][y+1] == c ? 1 : 0
	};
	int w_diag = 1,
		w_horiz = 2,
		w_vert  = 2;
/*
	0 1 2
	3 4 5
	6 7 8

	for(int i=0; i <= 8; i++)
	{
		if(g[i] && i != 4)
		print(i<< " ");
	}
*/
	int local = g[4] * (
			w_vert  * ( g[1] + g[7] ) +
			w_diag  * ( g[0] + g[2] + g[6] + g[8] ) +
			w_horiz * ( g[3] + g[5] )
		);
	return local;
}

int fitness()
{
	int k_vert = 0;
	int k_horiz = 0;

	// look for horizontal consecutives.

	for(int i = 1; i <= 9; i++)
	{
		int k = 0, l = 0;
		for(int j = 1; j <= 9; j++)
		{
			
			if( field[i][j] == empty )
			{
				if( k >= 5 ) k *= k;
				k_vert += k * k * k;
				k = 0;
			}
			else if( field[i][j] == l )
			{
				k++;
			}
			else
			{
				//k_vert -= k * k;
				k = 0;
			}
			l = field[i][j];
		}
		k_vert += k * k;
	}

	// look for vertical consecutives
	for(int j=1; j <= 9; j++)
	{
		int k = 0, l = 0;
		for(int i=1; i <= 9; i++)
		{
			if( field[i][j] == empty )
			{
				if( k >= 5 ) k *= k;
				k_vert += k * k * k;
				k = 0;
			}
			else if( field[i][j] == l )
			{
				k++;
			}
			else if( l == 0 )
			{
				k++;
			}
			else
			{
				//k_vert -= k * k;
				k = 0;
			}
			l = field[i][j];
		}
		k_horiz += k * k * k;
	}
	
	//k_horiz *= k_horiz;
	//k_vert  *= k_vert;


	//if( k_horiz > k_vert ) k_vert = k_horiz;
	return k_vert + k_horiz;

}



int findpath(int sx, int sy, int dx, int dy)
{
	int cx = dx, cy = dx;
	int dir = 0, xx = 0, yy = 1;

	int t;
	for( t = 0; t < 10000; t++ )
	{
		switch( dir )
		{
			case  0 :  xx =  1 ; yy =  0 ; break;
			case  1 :  xx =  0 ; yy =  1 ; break;
			case  2 :  xx = -1 ; yy =  0 ; break;
			case  3 :  xx =  0 ; yy = -1 ; break;
		}
		if( field[cx+xx][cy+yy] != empty )
		{
			dir++;
		}
		else
		{
			cx += xx; cy += yy;
		}
		if( cx == sx and cy == sy )
			return 1;
	}
	return 0;
}

void seekmove(int x, int y)
{
	int c = field[x][y];
	for(int i=1; i <= 9; i++)
	{
		for(int j=1; j <= 9; j++)
		{
			if( not ( i == x && j == y ) )
			{
				int local = localfit(i,j,c);
				int path = findpath(x,y,i,j);

				if( findpath > 0 && local > 0 )
				{
					int fit0 = fitness();

					field[x][y] = empty;
					field[i][j] = c;
					int fit = fitness() - fit0;
					field[i][j] = empty;
					field[x][y] = c;

					if( fit > 0)
					{
						//print("\t" << (char)('@'+j) << i << ": " << fit << "\n");
						moves[cm].dx = i;
						moves[cm].dy = j;
						moves[cm].val = fit + local;
						++cm;
						moves[cm].sx = moves[cm-1].sx;
						moves[cm].sy = moves[cm-1].sy;
					}
				}
			}
		}
	}
}

void printmove(int m)
{
	print(
		"\r\033[65C\033[1;3" << field[moves[m].sx][moves[m].sy] << "m" << 
		(char)('@' + moves[m].sy) << moves[m].sx << " : " << 
		(char)('@' + moves[m].dy) << moves[m].dx << " = " << moves[m].val
		<< "\033[0m\n";
	);
}

int compare(const void * e1, const void * e2)
{
	return ((move *)e2)->val - ((move *)e1)->val;
}

void domove()
{
	cm = 1;
	moves[cm-1].val = -1;
	for(int i=1; i <= 9; i++)
	{
		for(int j=1; j <= 9; j++)
		{
			if(movable(i,j)){
				moves[cm].sx = i;
				moves[cm].sy = j;
				seekmove(i,j);
			}
		}
		
	}

	int maxm, maxv;
	int m;

	qsort( moves, cm + 1, sizeof( move ), compare );
	
	print("\033[H");
	for(m = 0; m < 30 && m < cm; m++)
	{
		printmove(m);
	}
	usleep(5*TS);

	m = 0;

	field[ moves[m].dx ][ moves[m].dy ] = field[ moves[m].sx ][ moves[m].sy ];
	field[ moves[m].sx ][ moves[m].sy ] = empty;

}

void clearlines()
{
	int b, i, j, k, l;
	for(i = 1; i <= 9; i++)
	{
		k = 0, l = 0;
		for(j = 1; j <= 10; j++)
		{
			if( field[i][j] == empty || field[i][j] == edge )
			{
				if( k >= 5 ){
					for(b = 1; b <= k; b++)
					{
						field[i][j-b] = empty;
					}
				}
				k = 0;
			}
			else if( field[i][j] == l )
			{
				k++;
			}
			else 
			{
				k = 1;
			}
			l = field[i][j];
		}
	}

	for(j = 1; j <= 9; j++)
	{
		k = 0, l = 0;
		for(i = 1; i <= 10; i++)
		{
			if( field[i][j] == empty || field[i][j] == edge )
			{
				if( k >= 5 ){
					for(b=1; b <= k; b++)
					{
						field[i-b][j] = empty;
					}
				}
				k = 0;
			}
			else if( field[i][j] == l )
			{
				k++;
			}
			else
			{
				k = 1;
			}
			l = field[i][j];
		}
	}
	
}

int main(void)
{
	setupfield();
	addballs(5);
	printfield();

	int t;

	for(t=0;;t++){
		print("\033[2J");
		printfield();
		domove();
		usleep(TS);
		printfield(); 
		usleep(2*TS);
		clearlines();
		printfield(); usleep(TS);
		if( not addballs(3) )
			break;
		printfield(); usleep(TS);
	}
	print("\n" << t << " turns\n");

	print("\033[40;37;0m\n");

	return 0;
}
