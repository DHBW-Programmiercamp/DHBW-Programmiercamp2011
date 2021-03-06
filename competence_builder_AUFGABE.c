/*  Competence Builder.c
    Competence Builder - AI Programmiercamp 2011, DHBW-Mannheim
	Team: Tux on the Fly
	Members: Timo Jacobs, Felix, Otto, Stephan Pfistner, Dirk Klostermann, Nicolai Ommer, Daniel Andres Lopez
	Releasedate: 4.5.2011
  	Licensed under BSD license. See LICENSE.txt
*/

// Header files
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#ifdef _WIN32 // _WIN32 is defined by many compilers available for the Windows operating system, but not by others.
#include "C:\MinGW\include\SDL\SDL.h"
#else
#include "SDL/SDL.h"
#endif

#define TILE_BACKGROUND 5
#define TILE_FLOOR 6
#define TILE_TABLE 4
#define MIN_PLAYER_X  2*Size_tile
#define MENU_BG_SIZE 95
#define MENU_BUTTON_WIDTH 600
#define MENU_BUTTON_HEIGHT 50
#define MENU_RANDOM_WIDTH 300
#define MENU_HEADER_WIDTH 600
#define MENU_HEADER_HEIGHT 100
#define MENU_TUX_HEIGHT 78
#define MENU_TUX_WIDTH 102
#define MENU_HELP_WIDTH 600
#define MENU_HELP_HEIGHT 200
#define T_TUX_SIZE 32
//Set player 20px above REAL floor
#define CHARACTER_FLOOR Win_floor_y
#define KI_rows 12
#define KI_cells 14

// Graphics data - SDL variables
SDL_Surface *graphics, *screen, *menu_bg, *menu_button, *menu_header, *menu_tux, *thousand_tux, *menu_random, *menu_help;  // Graphics data, screen data, menu data

// Auto-Control
int auto_control_val = 1;	//KI ist aktiviert

// Definition of sizes of Graphic elements, according to graphics in competence_builder.bmp
const int Size_comp=50;                       // Size (x==y) of competence element
const int Size_tile=96;                       // Size (x==y) of player and background graphics
const int Size_digit_x=40, Size_digit_y=52;   // Size of large digits for number display
const int Size_smalldigit_x=12, Size_smalldigit_y=21;   // Size of small digits for number display

// Basic data of Window layout and physics
// Better not change this data -> game behaviours should be comparable in competition
const int Win_width=1056, Win_height=760;     // screen resolution
const int Win_floor_y=630;                    // Position of floor
const int Element_start_x=100;                // Start position of element
const float Player_v_x=8.0f;                 // Player speed
const float Gravity=0.03f;                   // Gravity 


// Competence element
typedef struct {
   float x, y;            				// Position
   float vx, vy;         				// Speed
   char comp;            				// Type of competence 0 - 3,  crash=4, deleted 5
   unsigned short int  points;	  		//Points per element
   unsigned short int  countdown;	    //counts down to 0 if a block explodes -> explode vanish
} element_type;

// Player data
typedef struct {
	float x, y;           // Position
	int steps;            // Steps for Player Animation
} player_data_type;

// Current game information/state - current level data
typedef struct {
	// Data to be read from file
	int levels;              // Number of game levels
	int *element_pause;       // Array (for levels) of time interval between elements
	int *cur_max;             // Array (for levels) of number of curriculum elements
	char *curriculum;        // Array of competence numbers (0-3) [for each level this is filled new]
	  
	// Current game state
	int cur_level;           // Current level
	element_type *element;   // array of competence elements
	int element_countdown;   // Countdown until next element is launched
	int cur_act;             // Current element 
	unsigned int all_points; // Gesamtpunktzahl

	//KI Variables
	int just_thrown;
	int toRun;				//wohin die KI rennen muss
	short comps[KI_rows][KI_cells];
} game_state_type;


/*************************************************************************************
 * Helper
 */

int digit_count(int number) {
	int i, digits;
	digits = 1; //Always 1 digit
	for(i = 10; i < number; i *= 10, digits++);
	return digits;
}

/*************************************************************************************
 * Initialization and load data
 */

// Load lecture/level plan from file
//Load all level definition data
int load_game_data_info(game_state_type *g, char *filename) {
	FILE *file;
	short int i_level=0; 	//  Counter for levels

    // Open the file for reading
    file = fopen(filename,"r");
    if(file==NULL) {
        printf("Error: can't open file.\n");
        return -1;
    }
    
	fscanf(file,"%d",&g->levels);		// Read numbers of game levels

	g->cur_max = (int *)malloc(sizeof(int) * g->levels);  //Malloc to generate Array for cur_max
	g->element_pause = (int *)malloc(sizeof(int) * g->levels);  //Malloc to generate Array for element_pause

	for(i_level=0; i_level < g->levels; i_level++){
		fscanf(file,"%d %d ",&(g->cur_max[i_level]), &(g->element_pause[i_level]) );		// Read for each level: cur_max and element_pause
	}
    fclose(file);    // close file
    return 0;
}

//Load specific level data
int load_game_data(game_state_type *g, char *filename) {
	FILE *file;
	short int cur_count=0; 	// Zähler für das Einlesen des Curriculums
	int help2=0, i=0; //Zwischenspiecher um übersprungene Daten ins Leere laufen zu lassen  (Funktion für fscanf fehlt: Springe zu einer bestimmten Zahl)
	int help=0;

	// Open the file for reading
	file=fopen(filename,"r");
	if(file==NULL) {
		printf("Error: can't open file.\n");
		return -1;
	}

	g->curriculum = (char *)malloc(sizeof(char) * g->cur_max[(g->cur_level)]);  //Speicher für Array curriculum allokieren

	help2 = 1 + 2 * (g->levels);
	for(i=0;i < g->cur_level;i++) {
		help2 = help2 + g->cur_max[i];
	}
	for(i=0;i<help2;i++) {
		fscanf(file,"%d ", &help );		// Lese aus File uninteressante Blöcke und schreibe in helper Variable die nicht weiter benutzt wird...
		if (help == '\n' || help == '\r')
			help2++;
	}

	for(cur_count=0;cur_count<(g->cur_max[(g->cur_level)]);cur_count++) { 		//Schleife um alle Blöcke  nacheinander auszulesen
		fscanf(file,"%c ",&(g->curriculum[cur_count]) );		// Lese aus File Blöcke...
		g->curriculum[cur_count] = g->curriculum[cur_count] - 48; // ASCII Konvertieren
	}

    fclose(file);    // close file
    return 0;
}

int load_game_data_random(game_state_type *g) {
	short int cur_count=0;

    srand(time(0));

    free(g->cur_max);
	free(g->element_pause);

	g->levels = 1;
    g->cur_max = (int *)malloc(sizeof(int));  //Malloc to generate Array for cur_max
    g->cur_max[0] = (rand()%100 + 1);
	g->element_pause = (int *)malloc(sizeof(int));  //Malloc to generate Array for element_pause
    g->element_pause[0] = ((rand()%15+5) * 100);

	g->curriculum = (char *)malloc(sizeof(char) * g->cur_max[0]);  //Speicher für Array curriculum allokieren

	for(cur_count=0; cur_count < (g->cur_max[0]); cur_count++) {
		g->curriculum[cur_count] = rand()%4;
	}

    return 0;
}

// Initialize next level / player
void init_level(game_state_type *g, player_data_type *p ) {
	g->element = (element_type *)malloc(sizeof(element_type)*g->cur_max[(g->cur_level)]);   // Malloc Array for element
	g->element_countdown = g->element_pause[(g->cur_level)] / 20; //1200ms /20ms (20ms braucht ca. ein Programmdurchlauf)
	g->cur_act  =0;
	g->just_thrown = 1;
	g->all_points = 0;
	g->toRun = 900;

	int i,j;
	for(i=0; i<KI_rows; i++)
		for(j=0; j<KI_cells;j++)
			g->comps[i][j]=-1;

	// Init player position
	p->x = Win_width-Size_tile;
	p->y = (float)CHARACTER_FLOOR;
}

/***************************************************************************
 * Main game loop: Logic
 ***************************************************************************/
void init_next_element(game_state_type *g, player_data_type *pl) {
	g->element_countdown--;
	if(g->element_countdown == 0 && g->cur_act!=g->cur_max[(g->cur_level)]) {//Soll er gerade werfen und sind noch Elemente zum Werfen da
		element_type el;

		g->element_countdown = g->element_pause[(g->cur_level)] / 20;
		// The teacher throws in such direction that the competence lands at the current player position
		// For the competition these calculations must be left unchanged!
		el.x=(float)Element_start_x;
		el.y=(float)Win_floor_y;
		el.vy=-(float)sqrt(el.y * 2. * Gravity);
		el.vx=(float)((pl->x-el.x)/(-el.vy/Gravity*2.));
		el.comp = g->curriculum[g->cur_act];
		el.points = 0;
		g->element[g->cur_act]=el;
		//g->element[g->cur_act]=*el;
		g->cur_act++;
		g->just_thrown=1;
	}

	// Just in case, some notes regarding the physics - you don't need to understand that
    // h max = v2 / (2g)       v=sqrt(h max * 2 * g)
	// R = v^2 / g * sin (2 beta)
	// R * g = v2 * sin b * cos b
	// Steigzeit: v0/g
}

void explode(element_type *el) {
	el->comp = 4;
	el->vx=0;
	el->vy=0;
	el->countdown=50;
	el->points=0;
}

/********************************************************************
 * collision check  and set points                                  *
 * return:     //0 no coll     //2 bridge     //3 on top            *
 ********************************************************************/

int check_collision(element_type *el1, element_type *el2, game_state_type *g) {
	int i;
	char bridge;
	const float buffer=6.0;
	element_type *el3;

	//sidecollision check
	if(el1->x + Size_comp > el2->x && el1->x + Size_comp - buffer < el2->x &&
			el1->y + Size_comp > el2->y && el1->y < el2->y + Size_comp) {
		el1->vx=(-0.9)*el1->vx;
	}
	//topcollision check
	else if(el1->x + Size_comp > el2->x && el1->x < el2->x + Size_comp) {
		if(el1->y + Size_comp > el2->y && el1->y + Size_comp < el2->y + buffer) {
			// find another block below (bridge between two blocks)
			i=0;
			bridge=0;
			while(i<g->cur_act && bridge==0) {
				if ((el1->x + Size_comp >= g->element[i].x) &&
						(el1->x <= g->element[i].x + Size_comp) &&
						el1->y + Size_comp > g->element[i].y &&
						el1->y + Size_comp < g->element[i].y + buffer &&
						&(g->element[i])!= el1 && &(g->element[i])!= el2)
				{
					bridge=1;
					el3=&(g->element[i]);
				}
				i++;
			}

			if (bridge) {
				if(el1->points==0) {
					if(el1->comp!=el2->comp) el1->points+=el2->points;
					if(el1->comp!=el3->comp) el1->points+=el3->points;
					if(el1->comp!=el2->comp && el1->comp!=el3->comp && el1->points==0) el1->points=1;
				}
				return 2; //top collision
			}
			else {
				// check if element is not more than half on the element below
				if(el1->x + Size_comp/2 < el2->x || el1->x > el2->x + Size_comp/2) {
					explode(el1);
				}
				else {
					if(el1->comp!=el2->comp  && el1->points==0) el1->points=el2->points;
					if(el1->comp!=el2->comp  && el1->points==0 && el2->points==0) el1->points=1;
					return 3; //bridge collision
				}
			}
		}
		else if(el1->y < el2->y + Size_comp + buffer && el1->y > el2->y + Size_comp) {
			explode(el1);
		}
	}
	return 0; //no collision
}

/********************************************************************
 * Update/move the existing competence element(s)                   *
 ********************************************************************/
void move_elements(game_state_type *g) {
	int i,j,coll,k;
	for(i=0;i<g->cur_act;i++) {
		if(g->element[i].comp==4) {
			if(g->element[i].countdown!=0) {
				g->element[i].countdown--;
			}
			else {
				// move element outside screen
				g->element[i].x = Win_width - Size_comp;
				g->element[i].y = Win_height - Size_comp;
				g->element[i].comp = 5;
			}
		}
		else if(g->element[i].y<=Win_floor_y) { // Competence does not hit floor yet
			// Check for collision with other elements
			j=-1;
			do {
				j++;
				if(j!=i) {
					coll = check_collision(&g->element[i],&g->element[j],g);
				}
				else {
					coll=0;
				}
			} while(j<g->cur_act && coll==0);

			if(coll==0) {
				// Normal motion based on current speed and gravity.
				g->element[i].vy+=Gravity;   // Gravity affects vy
				g->element[i].y+=g->element[i].vy;     // change position based on speed
				g->element[i].x+=g->element[i].vx;
			}
			else {
				g->all_points = 0;
				for (k=0;k<g->cur_act;k++) {
					g->all_points += g->element[k].points;  //count all points from cur_act
				}
			}
		}
		else if(g->element[i].x<MIN_PLAYER_X) {
			explode(&g->element[i]);
		}
		else if (g->element[i].comp<4){
			g->element[i].points=1;  //set groundelement.points 1
			g->all_points = 0;
			for (k=0;k<g->cur_act;k++) {
				g->all_points += g->element[k].points;  //count all points from cur_act
			}
		}
	}
}
    
/*******************************************************************
 * Functions using the SDL-Graphics-library                        *
 * for drawing the game graphics on the screen                     *
 * For the 'must'-requirements you don't need to change            *
 * or add anything here                                            *
 *******************************************************************/
// Initialize the SDL-library and load game graphics
int init_SDL() {
	Uint32 color;

    // Initialize SDL 
    if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0 ) {     // SDL_INIT_AUDIO|
        printf("Unable to init SDL: %s\n", SDL_GetError()); exit(1);
    } 
    atexit(SDL_Quit);
    screen = SDL_SetVideoMode(Win_width, Win_height, 32, SDL_DOUBLEBUF | SDL_HWSURFACE);
	if(!screen) {
        printf("Unable to init video: %s\n", SDL_GetError()); exit(1);
    }	
	// Load graphics
	SDL_WM_SetCaption("Tux on the fly: Competence Builder", 0); //set Programmtitle
	SDL_WM_SetIcon(SDL_LoadBMP("Tux_icon.bmp"), 0); //set BMP as Icon
	graphics = SDL_LoadBMP("competence_builder.bmp");
	menu_bg = SDL_LoadBMP("menu_texture.bmp");
	menu_button = SDL_LoadBMP("menu_buttons.bmp");
	menu_header = SDL_LoadBMP("menu_header.bmp");
	menu_tux = SDL_LoadBMP("flying_tux.bmp");
	thousand_tux = SDL_LoadBMP("Tux_icon_32x32.bmp");
	menu_random = SDL_LoadBMP("zufall.bmp");
	menu_help = SDL_LoadBMP("tasten.bmp");

    if (!graphics || !menu_bg || !menu_button || !menu_header || !menu_tux || !menu_random || !menu_help) {
	    printf("Unable to load bitmaps: %s\n", SDL_GetError());  exit(1);
	}
	// Set transparency color
	color = SDL_MapRGB(screen->format, 255, 200, 200);
	SDL_SetColorKey(graphics, SDL_SRCCOLORKEY | SDL_RLEACCEL, color);
	graphics = SDL_DisplayFormatAlpha(graphics);
	SDL_SetColorKey(menu_header, SDL_SRCCOLORKEY | SDL_RLEACCEL, color);
	menu_header = SDL_DisplayFormatAlpha(menu_header);
	SDL_SetColorKey(menu_tux, SDL_SRCCOLORKEY | SDL_RLEACCEL, color);
	menu_tux = SDL_DisplayFormatAlpha(menu_tux);
	SDL_SetColorKey(thousand_tux, SDL_SRCCOLORKEY | SDL_RLEACCEL, color);
	thousand_tux = SDL_DisplayFormatAlpha(thousand_tux);

	return 0;
}

// Helper function to draw a rectangle with RGB-color
void draw_rect(int x, int y, int w, int h, int r, int g, int b) {
	SDL_Rect rct;
	Uint32 color;
	
	rct.x=x; rct.y=y; rct.w=w; rct.h=h;
	color=SDL_MapRGB(screen->format, r, g, b);
	SDL_FillRect(screen, &rct, color);
}

// draw graphics for student/teacher/background at position (x, y) (number=0-6) 
void draw_tile(int x, int y, int number) {
	SDL_Rect src, dest;

	//Set sizes
	src.w = dest.w = Size_tile;
	src.h = dest.h = Size_tile;
	src.x = number*Size_tile;
	src.y = 0;
	dest.x = x;
	dest.y = y;

	SDL_BlitSurface(graphics, &src, screen, &dest);
}

// draw image
void draw_image(SDL_Surface *surface, int x, int y, int w, int h)
{
	SDL_Rect src, dest;

	//Set sizes
	src.w = dest.w = w;
	src.h = dest.h = h;
	dest.x = x;
	dest.y = y;
	src.x = 0;
	src.y = 0;


	SDL_BlitSurface(surface, &src, screen, &dest);
}

void draw_button(int x, int y, int number)
{
	SDL_Rect src, dest;

	src.w=dest.w=MENU_BUTTON_WIDTH;
	src.h=dest.h=MENU_BUTTON_HEIGHT;
	src.x = 0;	src.y = number*MENU_BUTTON_HEIGHT;
	dest.x = x;	dest.y = y;
	SDL_BlitSurface(menu_button, &src, screen, &dest);
}

void draw_zufall(int x, int y, int number)
{
	SDL_Rect src, dest;

	src.w=dest.w=MENU_RANDOM_WIDTH;
	src.h=dest.h=MENU_BUTTON_HEIGHT;
	src.x = 0;	src.y = number*MENU_BUTTON_HEIGHT;
	dest.x = x;	dest.y = y;
	SDL_BlitSurface(menu_random, &src, screen, &dest);
}

// draw competence elemtents at position (x, y) (number=0-3)
void draw_competence(int x, int y, int number) {
	SDL_Rect src, dest;

	src.w=src.h=dest.w=dest.h=Size_comp;
	src.x = number*Size_comp;	src.y = Size_tile;
	dest.x = x;	dest.y = y;
	SDL_BlitSurface(graphics, &src, screen, &dest);
}

// Draw digit 'number' (size==1: large, otherwise: small)
void draw_digit(int x, int y, int number, char size) {
	SDL_Rect src, dest;

	src.w=dest.w=(size? Size_digit_x : Size_smalldigit_x);
	src.h=dest.h=(size? Size_digit_y : Size_smalldigit_y);
	src.x = number*src.w; 
	src.y=Size_tile+Size_comp+(size? 0:Size_digit_y);
	dest.x = x;	dest.y = y;

	SDL_BlitSurface(graphics, &src, screen, &dest);
}

// Draw Global Score
void draw_globalscore(int x, int y, int number) {
	SDL_Rect src, dest;

	src.x = number*Size_digit_x;
	src.y = Size_comp + Size_tile;
	src.w = dest.w = Size_digit_x;
	src.h = dest.h = Size_digit_y;
	dest.x = x;	dest.y = y;

	SDL_BlitSurface(graphics, &src, screen, &dest);
}

// Draw numbers on blocks
void draw_blockscore(int x, int y, int number) {
	SDL_Rect src, dest;

	src.y = Size_digit_y + Size_comp + Size_tile;
	src.x = number*Size_smalldigit_x;
	src.w = dest.w = Size_smalldigit_x;
	src.h = dest.h = Size_smalldigit_y;
	dest.x = x;	dest.y = y;

	SDL_BlitSurface(graphics, &src, screen, &dest);
}

void draw_sitebar(int x, int y, int number) {
	SDL_Rect src, dest;

	src.w=src.h=dest.w=dest.h=Size_comp/2;
	src.x = number*Size_comp+Size_comp/2;	src.y = Size_tile;
	dest.x = x;	dest.y = y;
	SDL_BlitSurface(graphics, &src, screen, &dest);
}

/*******************************************************************
 * Refresh the screen and draw all graphics                        *
 *******************************************************************/
void paint_menu(int *tux_x, int *random)
{
	int x, y;
	//Draw colored background
	draw_rect(0,Win_floor_y,Win_width, Win_height-Win_floor_y, 139, 69, 19);

	//Draw background color
	for(y = 0; y < Win_height; y += MENU_BG_SIZE)
		for(x = 0; x < Win_width; x += MENU_BG_SIZE)
			draw_image(menu_bg, x, y, MENU_BG_SIZE, MENU_BG_SIZE);

	//Header
	draw_image(menu_header, round(Win_width/2)-round(MENU_HEADER_WIDTH/2), 20, MENU_HEADER_WIDTH, MENU_HEADER_HEIGHT);

	//Draw Buttons
	draw_button(round(Win_width/2)-round(MENU_BUTTON_WIDTH/2), 300, 0); //Normal
	draw_button(round(Win_width/2)-round(MENU_BUTTON_WIDTH/2), 370, 1); //Automatik
	draw_button(round(Win_width/2)-round(MENU_BUTTON_WIDTH/2), 440, 2); //Beenden

	//Tux
	draw_image(menu_tux, (*tux_x), 130, MENU_TUX_WIDTH, MENU_TUX_HEIGHT);
	(*tux_x) = ((*tux_x)+10)%Win_width;

	//Random Button
	draw_zufall(round(Win_width/2)-round(MENU_BUTTON_WIDTH/2), 230, 1-(*random));

	//Help
	draw_image(menu_help, round(Win_width/2)-round(MENU_HELP_WIDTH/2), 510, MENU_HELP_WIDTH, MENU_HELP_HEIGHT);


	SDL_Flip(screen);
}

void paint_all(game_state_type *g, player_data_type *pl, int key_x) {
    if(!g)
    	return;
	
    int x, y, i, tmpscore, num;
    char nl;

    //Draw color background
    draw_rect(0,Win_floor_y,Win_width, Win_height-Win_floor_y,  30,30,50);

	//Draw background
	for(y = 0; y < Win_floor_y; y += Size_tile)
		for(x = 0; x < Win_width; x += Size_tile)
			draw_tile(x, y, TILE_BACKGROUND);

	//Draw teachers place
	for(x = 0; x < MIN_PLAYER_X; x += Size_tile)
		draw_tile(x, Win_floor_y, TILE_FLOOR);

	//Draw tables
	for(; x < Win_width; x += Size_tile)
		draw_tile(x, Win_floor_y, TILE_TABLE);

	//Draw teacher with movement
	if(g->element_countdown<10) {
		draw_tile(50, CHARACTER_FLOOR, 2 );
	}
	else {
		draw_tile(50, CHARACTER_FLOOR, 3 );
	}

	// Draw list of remaining competences in curriculum
	int y_sitebar=Win_floor_y+Size_tile-Size_comp/2;
	for(i = g->cur_act; i < g->cur_max[g->cur_level]; i++) {
		draw_sitebar(0,y_sitebar,g->curriculum[i]);
		y_sitebar-=Size_comp/2;
	}

	//Draw global score
	//Go Numbers downwards
	tmpscore = g->all_points;
	for(x = 0, i = 1000; i > 0; i = floor((float)i/10), x++) {
		num = floor(tmpscore/i);
		draw_globalscore(Size_digit_x*x, 0, num);
		tmpscore -= i*num;
	}

	// Draw elements + points
	for(i = 0; i < g->cur_act; i++) {
		if(g->element[i].comp != 5) { //Do not draw not existing curriculum
			draw_competence(g->element[i].x, g->element[i].y, g->element[i].comp);
			int digits = digit_count(g->element[i].points);
			//Position y middle, x middle in box
			int left_x, middle_x, middle_y, up_y;
			middle_y = round((float)Size_smalldigit_y/2);
			up_y = g->element[i].y + (round(Size_comp/2) - middle_y);
			middle_x = round((digits*Size_smalldigit_x)/2);
			left_x = g->element[i].x + (round(Size_comp/2) - middle_x);
			nl = 0; //Did we have a valid number (so draw also 0)?
			tmpscore = g->element[i].points;
			//<HackyAsHell>
			switch(digits) {
				case 3: //>100
					draw_blockscore(left_x, up_y, floor(tmpscore/100));
					tmpscore -= floor(tmpscore/100)*100;
					draw_blockscore(left_x+Size_smalldigit_x, up_y, floor(tmpscore/10));
					tmpscore -= floor(tmpscore/10)*10;
					draw_blockscore(left_x+2*Size_smalldigit_x, up_y, tmpscore);
					break;
				case 2:
					draw_blockscore(left_x, up_y, floor(tmpscore/10));
					tmpscore -= floor(tmpscore/10)*10;
					draw_blockscore(left_x+Size_smalldigit_x, up_y, tmpscore);
					break;
				case 1:
					draw_blockscore(left_x, up_y, tmpscore);
					break;
				default: { //>1000
					int o = round(Size_comp/2) - round(T_TUX_SIZE/2);
					draw_image(thousand_tux, g->element[i].x + o, g->element[i].y + o, T_TUX_SIZE, T_TUX_SIZE);
					break;
				}
			}
			//</HackyAsHell>
		}
	}

	//Draw player
	draw_tile((int)pl->x, CHARACTER_FLOOR, (key_x == 0 ? 1 : (pl->steps%4==0 ? 1 : 0)));

	// Refresh screen (double buffering)
	SDL_Flip(screen);
}


/********************************************************************
 * SDL-Function to check for keyboard events                        *
 ********************************************************************/
int process_menu_click(char *left_clicked, int *random)
{
	SDL_PumpEvents();

	int x, y;
	if(SDL_GetMouseState(&x, &y)&SDL_BUTTON(1))
		(*left_clicked)=1;
	else if((*left_clicked)) {//was pressed, is not now
		//is in button x bounds
		int x1 = round(Win_width/2)-round(MENU_BUTTON_WIDTH/2);
		if (x >= x1 && x <= x+MENU_RANDOM_WIDTH) {
					printf("X\n");
					if(y >= 230 && y <= 280) { //Random
						printf("Y\n");
						(*random) = 1-(*random);
					}
				}
		if(x >= round(Win_width/2)-round(MENU_BUTTON_WIDTH/2) && x <= round(Win_width/2)+round(MENU_BUTTON_WIDTH/2)) {
			if(y >= 300 && y <= 350) { //Manuell
				return 2;
			} else if(y >= 370 && y <= 420) { //Automatic
				return 3;
			} else if(y >= 440 && y <= 490) { //Close
				return 1;
			}
		}

		(*left_clicked)=0;
	}
	SDL_Delay(10); //stop constant polling
	return 0;
}

int key_control(int *key_x, int *key_c) {
	SDL_Event keyevent;

	// Verbesserung von Timo: immer +1 / -1, so k�nnen auch kurzzeitig rechts und links gleichzeitig gedr�ckt werden und es ist trotzdem intuitiv

	SDL_PollEvent(&keyevent);
	if(keyevent.type==SDL_KEYDOWN) {
        switch(keyevent.key.keysym.sym) {
           case SDLK_LEFT: (*key_x)--; break;
           case SDLK_RIGHT: (*key_x)++; break;
           case 'a':
        	   auto_control_val = !auto_control_val;
        	   *key_x=0; break;
           case 's': *key_c=1; break;	// slower
           case 'f': *key_c=2; break;	// faster
           case 'p': *key_c=3; break;	// pause
           case 'r': *key_c=4; break;	// resume
           case 'n': *key_c=5; break;	// next level
           case 'z': *key_c=6; break;	// random
           case SDLK_ESCAPE: exit(0); break;

           default: break;
		}
	}

	else if(keyevent.type==SDL_KEYUP) {

		switch(keyevent.key.keysym.sym) {

		case SDLK_LEFT: (*key_x)++;	break;
		case SDLK_RIGHT: (*key_x)--; break;
        case 's':
        case 'f':
        case 'p':
        case 'r':
        case 'n':
        case 'z':
     	   *key_c=0;
     	   break;

		default: break;
		}
	}
	return 1;
}

/********************************************************************
 * The student-robot  - KI                                              *
 ********************************************************************/

int auto_control(game_state_type *g, player_data_type *pl)
{

	// erste KI: F�r niedrigere Geschwindigkeiten

		/**Comps:
		 * speichert wie die existierenden Comps zueinander stehen (sollten)
		 * der Algorithmus beginnt oben rechts und geht Spalte für Spalte durch und
		 * setzt die Comp an den ersten passenden Ort (keine selbe Comp dadrunter).
		 * ->Pyramide
		 * z.B.:
		 * ------------------
		 * |				| Pyramide:
		 * |			1	|	oben:    1
		 * |		2	0	|	unten: 2   0
		 * ------------------
		 */
		if(g->just_thrown==1) //dieser Teil wird nur direkt nach dem initialisieren eines neuen Elements benötigt
		{
			int nextx=-1,nexty=0;
			int nextcomp = g->curriculum[g->cur_act];
			int i,j;
			int cells = KI_cells;
			int rows = KI_rows;
			int grenze = 600;
			if (*(g->element_pause) <= grenze) {
				cells = 4;
			}
			for(i=rows; i>0; i--)//unterste Reihe auslassen
				for(j=0; j<cells-1;j++)//ganz linke Spalte auslassen
					if(g->comps[i][j]==-1&& g->comps[i-1][j+1]!=nextcomp && g->comps[i-1][j]!=nextcomp && nextx==-1 && g->comps[i-1][j]!=-1 && g->comps[i-1][j+1]!=-1)
					{
						nextx=j;
						nexty=i;
					}
			if(nextx==-1)//die untereste Reihe fehlt noch, hier wird ein anderes System benötigt: erst freie Position von rechts
			{
				int j=0;
				while(nextx==-1)
				{
					if(g->comps[0][j]==-1)
					{
						nextx=j;
						nexty=0;
					}
					j++;
					if (j == cells) {
						break;
					}
				}
			}
			if (*(g->element_pause) <= grenze) {
				for(i=KI_rows; i>0; i--)//unterste Reihe auslassen
						for(j=cells; j<KI_cells-1;j++)//ganz linke Spalte auslassen
							if(g->comps[i][j]==-1&& g->comps[i-1][j+1]!=nextcomp && g->comps[i-1][j]!=nextcomp && nextx==-1 && g->comps[i-1][j]!=-1 && g->comps[i-1][j+1]!=-1)
							{
								nextx=j;
								nexty=i;
							}
					if(nextx==-1)//die untereste Reihe fehlt noch, hier wird ein anderes System benötigt: erst freie Position von rechts
					{
						int j=0;
						while(nextx==-1)
						{
							if(g->comps[0][j]==-1)
							{
								nextx=j;
								nexty=0;
							}
							j++;
						}
					}
			}
			g->just_thrown=0;
			g->comps[nexty][nextx]=nextcomp;
			int nextxpix;
			if(nextx!=-1)
				nextxpix=920-(nextx*55)-(nexty*10);
			else
				nextxpix=nextx;
			if(nextxpix<MIN_PLAYER_X)
			{
				 srand(time(NULL));
				 g->toRun = 700+rand()%200;
			}
			else
			//int nextypix=Win_floor_y-50-(nexty*50);
				g->toRun = nextxpix; //needed_position(g, pl, nextxpix, nextypix);
		}
		int key=0;
		if(g->toRun<pl->x)
			key=-1;
		else if(g->toRun>pl->x)
			key=1;
		else
			key=0;
		return key;
}


/******************************************
 * Main Function
 ***************************************/

int main(int argc, char *argv[]) {
	// Major game variables: Player + game state
	player_data_type player;
	game_state_type game;
	char pause=0;
	int delay=20, random=0;

	// some other variables
	int i, key_x, key_c, tux_x;
	
	init_SDL();
    load_game_data_info(&game, "competence_builder.txt");  //Load Level Definition data

    char action, left_mouse=0;
    tux_x = 0;
    while(!(action = process_menu_click(&left_mouse, &random)))
    	paint_menu(&tux_x, &random);
	switch(action) {
		case 1: //Close
			return 0;
		case 2: //Start manuell
			auto_control_val=0;
			break;
		case 3: //Start automatik
			auto_control_val=1;
			break;
		default:
			break;
	}

    //For-Schleife für verschiedene Levels
    for(game.cur_level=0;game.cur_level<game.levels;game.cur_level++)     {
        if (random == 0)
        	load_game_data(&game, "competence_builder.txt");
        else {
        	game.cur_level=0;
        	load_game_data_random(&game);
        	random = 0;
        }
        init_level(&game, &player);
		// The main control loop
		// key_x initialisieren
		key_x = 0;
		key_c = 0;
		// Abbrechen, wenn key_control 0 zur�ckgibt.
		while(key_control(&key_x,&key_c)) {
			if(key_c==1)
				delay+=10;
			else if(key_c==2) {
				delay-=10;
				if(delay<10) delay=3;
			}
			else if(key_c==3)
				pause=1;
			else if(key_c==4)
				pause=0;
			else if(key_c==5)
				break; // next level
			else if(key_c==6) {
				random = 1;
				break; // random level
			}
			if(pause==0) {
				//Draw first
				paint_all(&game, &player, key_x);
				init_next_element(&game, &player);

				// move_elements is called ten times before the graphics are updated
				// Thus, a better simulation precision is achieved
				// without spending too much performance on (slow) graphics output
				for(i=0; i<10; i++)
					move_elements(&game);

				// Wenn die Automatik l�uft sich dieser auch bedienen
		if (auto_control_val) {
			key_x = 0;
			key_x=auto_control(&game, &player); // use only for 'robot player'
		}
				/* Nur bewegen, wenn
				 * - der Player innerhalb der Bewegungsreichweite ist
				 * - der Player rechts au�erhalb der Bewegungsreichweite ist und sich nach links bewegen m�chte
				 * - der Player links au�erhalb der Bewegungsreichweite ist und sich nach rechts bewegen m�chte
				 */
				if ((player.x + Size_tile <= Win_width && player.x >= MIN_PLAYER_X) || (player.x + Size_tile > Win_width && key_x < 0) || (player.x < MIN_PLAYER_X && key_x > 0)) {
					player.x+=round(key_x*Player_v_x);    // Calculate new x-position
					player.steps++;
					if(player.steps>100000) player.steps=0;
				}

				paint_all(&game, &player, key_x);  // Update graphics
				SDL_Delay(delay); // wait 20 ms
			}
//			if (game.cur_act == game.cur_max[game.cur_level]) {
//				SDL_Delay(3000); // wait 20 ms
//				break;  //End if all blocks are shot
//
//			}

		}
		free(game.curriculum); //Free up memory of Level Data Array
    }
    free(game.cur_max); //Free up memory of Level Definition (cur_max) Array
    free(game.element_pause); //Free up memory of Level Definition (element_pause) Array
    free(game.element); //Free up memory of Element Array


    //Clean up Surfaces
    SDL_FreeSurface(graphics);
    SDL_FreeSurface(menu_bg);
    SDL_FreeSurface(menu_button);
    SDL_FreeSurface(menu_header);
    SDL_FreeSurface(menu_tux);
    SDL_FreeSurface(thousand_tux);
    SDL_FreeSurface(menu_random);
    SDL_FreeSurface(menu_help);
    return 0;
}

