#include <math.h>
#include <stdio.h>
#include <time.h>

// ------------------------------------
// framebuffer for testing the routines
// ------------------------------------

#define RES_X 20
#define RES_Y 20

char framebuffer[RES_X * RES_Y];

void flush(){
	int i, j;
	for(i = 0; i < RES_Y; ++i){
		for(j = 0; j < RES_X; ++j)
			putchar(framebuffer[i * RES_X + j]);
		putchar('\n');
	}
}

void clear(char c){
	int i;
	for(i = 0; i < RES_X * RES_Y; ++i)
		framebuffer[i] = c;
}

void putPixel(int x, int y, char c){
	if(x >= 0 && x < RES_X && y >= 0 && y < RES_Y)
		framebuffer[y * RES_X + x] = c;
}

// ------------------------------------
// rendering routines
// trying to implement top left filling
// based on microsoft's rasterization
// rules
// ------------------------------------

typedef struct{
	float x, y;
}vert_t;

typedef	struct{
	float x, sx;
	unsigned int height;
}edge_t;

// transforming an float to a integer based on the raster conventions
unsigned int getRaster(float n){
	unsigned int ret = (unsigned int)n;
	return ret + ((n - (float)ret) > 0.5);
}

void constructEdge(edge_t *e, vert_t* top, vert_t* bot, unsigned int *y_top){
	
	if(bot->y == top->y){
		return;
	}
	
	float inverseHeight = 1.0 / (bot->y - top->y);
	e->sx = (bot->x - top->x) * inverseHeight;
	
	float pre = (float)(*y_top) + 0.5 - top->y;
	e->x = top->x + e->sx * pre;
}

void drawEdges(edge_t *le, edge_t *re, unsigned int *y){
	unsigned int height = le->height < re->height ? le->height : re->height;
	le->height -= height;
	re->height -= height;
		
	while(height--){
		unsigned int beg = getRaster(le->x);
		unsigned int end = getRaster(re->x);
			
		for(; beg < end; ++beg){
			putPixel(beg, *y, 'O');
		}
			
		// edge stepping
		le->x += le->sx;
		re->x += re->sx;
		++(*y);
	}
}


// triangle rasterizer
void drawTriangle(vert_t *v0, vert_t *v1, vert_t *v2){
	
	// sorting for y
	if(v1->y < v0->y){
		vert_t *temp = v0;
		v0 = v1;
		v1 = temp;
	}
	if(v2->y < v1->y){
		vert_t *temp = v1;
		v1 = v2;
		v2 = temp;
	}
	if(v1->y < v0->y){
		vert_t *temp = v0;
		v0 = v1;
		v1 = temp;
	}
	
	// getting the raster y coordinates of every point
	unsigned int iy[3];
	iy[0] = getRaster(v0->y);
	iy[1] = getRaster(v1->y);
	iy[2] = getRaster(v2->y);
	
	edge_t topBot, topMid, midBot;
	constructEdge(&topBot, v0, v2, &iy[0]);
	constructEdge(&topMid, v0, v1, &iy[0]);
	constructEdge(&midBot, v1, v2, &iy[1]);
	
	topBot.height = iy[2] - iy[0];
	topMid.height = iy[1] - iy[0];
	midBot.height = iy[2] - iy[1];
	
	// check if mid is left or right (midleft = 1, means true, the mid is on the left)
	int midleft = (v1->y - v0->y) * (v2->x - v0->x) > (v1->x - v0->x) * (v2->y - v0->y);
	
	// current y
	unsigned int y = iy[0];
	if(midleft){
		drawEdges(&topMid, &topBot, &y);
		drawEdges(&midBot, &topBot, &y);
	}else{
		drawEdges(&topBot, &topMid, &y);
		drawEdges(&topBot, &midBot, &y);
	}
}

// polygon rasterizer
void drawPolygon(vert_t *v, unsigned int n){
	
	// everything with less than 3 vertices has no area
	if(n < 2){
		return;
	}
	if(n == 3){
		drawTriangle(&v[0], &v[1], &v[2]);
	}
	
	// storing edge vertex informations
	int l_top = 0, l_bot, r_top, r_bot;						// indexing vertices by integers
	unsigned int l_top_iy, l_bot_iy, r_top_iy, r_bot_iy;	// left and right edge integer y's
	unsigned int i;											// iterator, later the y position
	
	// getting the top most vertex (lowest y)
	for(i = 1; i < n; ++i){
		if(v[i].y < v[l_top].y){
			l_top = i;
		}
	}
	
	// marking the edge starting points, i is for now the current line we are drawing at
	r_top = l_top;
	l_top_iy = r_top_iy = i = getRaster(v[l_top].y);
	
	// initialize the edges
	edge_t le, re; le.height = 0; re.height = 0;
	
	// draw the polygon
	do{
		
		// recalculate if left or right edge end is reached
		if(!le.height){
			
			// new edge vertices
			l_bot = l_top - 1;
			
			if(l_bot < 0){
				l_bot += n;
			}
			
			// height
			l_bot_iy = getRaster(v[l_bot].y);
			le.height = l_bot_iy - l_top_iy;
			
			constructEdge(&le, &v[l_top], &v[l_bot], &l_top_iy);
			
			// vertex stepping
			l_top = l_bot;
			l_top_iy = l_bot_iy;
			
		}
		
		if(!re.height){
			
			// new edge vertices
			r_bot = r_top + 1;
			
			if(r_bot >= n){
				r_bot -= n;
			}
			
			// height
			r_bot_iy = getRaster(v[r_bot].y);
			re.height = r_bot_iy - r_top_iy;
			
			constructEdge(&re, &v[r_top], &v[r_bot], &r_top_iy);
			
			// vertex stepping
			r_top = r_bot;
			r_top_iy = r_bot_iy;
			
		}
		
		if(!le.height && !re.height){
			return;
		}
		
		// drawing spans between edges
		drawEdges(&le, &re, &i);
		
	}while(l_bot != r_bot);
}

// ------------------------------------
// driver to test routines
// ------------------------------------

// drawing 30 000 polygons

int	main(){
	
	clear(' ');
	int i, frames;
	
	// starting profiling
	printf("profiling...\n");
	clock_t start = clock();
	for(frames = 0; frames < 10000; ++frames){
		
		// how many polygons
		for(i = 0; i < 10000; ++i){
			
			
			vert_t v[4];
			v[0].x = 6;		v[0].y = 7;
			v[1].x = 11;	v[1].y = 7;
			v[2].x = 11;	v[2].y = 10;
			v[3].x = 6;		v[3].y = 12;
			drawPolygon(v, 4);/**/
			
			
			v[0].x = 2;	v[0].y = 0;
			v[1].x = 4;	v[1].y = 3;
			v[2].x = 1;	v[2].y = 3;
			drawPolygon(v, 3);/**/
			
			
			v[0].x = 9;		v[0].y = 0;
			v[1].x = 11;	v[1].y = 5;
			v[2].x = 7;		v[2].y = 3;
			drawPolygon(v, 3);/**/
			
		}
	}
	
	// printing metadata
	printf("%f ms per frames\n", (float)(clock() - start) / CLOCKS_PER_SEC * 1000 / frames);
	flush();
	printf("type to end...\n");
	getchar();
	return 0;
}
