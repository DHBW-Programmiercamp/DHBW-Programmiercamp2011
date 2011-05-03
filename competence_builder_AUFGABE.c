/*  Competence Builder.c
    Competence Builder - AI Programmiercamp 2011, DHBW-Mannheim

    Programmrumpf
*/

// Header files
#include <stdio.h>
#include <stdlib.h>
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
//Set player 20px above REAL floor
#define CHARACTER_FLOOR Win_floor_y

// Graphics data - SDL variables
SDL_Surface *graphics, *screen;  // Graphics data, screen data

// Wenn die Autimatik l�uft
int auto_control_val = 0;

// Definition of sizes of Graphic elements, according to graphics in competence_builder.bmp
const int Size_comp=50;                       // Size (x==y) of competence element
const int Size_tile=96;                       // Size (x==y) of player and background graphics
const int Size_digit_x=40, Size_digit_y=52;   // Size of large digits for number display
const int Size_smalldigit_x=12, Size_smalldigit_y=21;   // Size of small digits for number display

// Basic data of Window layout and physics
// Better not change this data -> game behaviours should be comparable in competition
const int Win_width=1024, Win_height=760;     // screen resolution
const int Win_floor_y=630;                    // Position of floor
const int Element_start_x=100;                // Start position of element
const float Player_v_x=8.0f;                 // Player speed
const float Gravity=0.03f;                   // Gravity 


// Important data structures, TODO: change/extend if needed

// Competence element
typedef struct {
   float x, y;            // Position
   float vx, vy;          // Speed
   char comp;             // Type of competence 0 - 3,  crash=4, deleted 5
   unsigned short int  points;	  //Points per elemnt
   unsigned short int  countdown;	  //counts down to 0 if a block explodes -> explode vanish
   // TODO: Extend if needed
} element_type;

// Player data
typedef struct {
	float x, y;           // Position
	int steps;            // TODO: You can use this for animating the moving player
} player_data_type;

// Current game information/state
typedef struct {
	// Data to be read from file
	int levels;              // Number of game levels
	int element_pause;       // Time interval between elements
	int cur_max;             // Number of curriculum elements
	char *curriculum;        // Array of competence numbers (0-3)
	  
	// Current game state
	element_type *element;   // array of competence elements -> TODO: use malloc
	int element_countdown;   // Countdown until next element is launched
	int cur_act;             // Current element 
	unsigned int all_points; // Gesamtpunktzahl
	int just_thrown;		//wichtig für die KI
	int toRun;				//wohin die KI rennen muss
	short comps[9][10]; // für die KI
} game_state_type;


/*************************************************************************************
 * Helper
 */

int digit_count(int number)
{
	int i, digits;
	digits = 1; //Always 1 ditgit
	for(i = 10; i < number; i *= 10, digits++);
	return digits;
}

/*************************************************************************************
 * Initialization
 */

// Load lecture/level plan from file
int load_game_data(game_state_type *g, char *filename)
{
	FILE *file;
	short int cur_count=0; 	// Zähler für das Einlesen des Curriculums

    // Open the file for reading
    file=fopen(filename,"r"); 
    if(file==NULL) {
        printf("Error: can't open file.\n");
        return -1;
    }
    
    // TODO: Read level data, allocate memory if needed
    
	fscanf(file,"%d",&g->levels);		// Lese aus File erste Zahl: Level Anzahl
	fscanf(file,"%d %d",&g->cur_max, &g->element_pause);		// Lese aus File Anzahl Blöcke, Wartezeit für Block

	g->curriculum = (char *)malloc(sizeof(char) * g->cur_max);  //Seicher für Array curriculum allokieren

	fscanf(file,"\r");		// Lese aus File: Zeilenumbruch

	for(cur_count=0;cur_count<(g->cur_max);cur_count++) { 		//Schleife um alle Blöcke  nacheinander auszulesen
		fscanf(file,"%c ",&(g->curriculum[cur_count]) );		// Lese aus File Blöcke...
		g->curriculum[cur_count] = g->curriculum[cur_count] - 48; // ASCII Konvertieren
	}

    fclose(file);    // close file
    return 0;
}

// Initialize next level / player
void init_level(game_state_type *g, player_data_type *p )
{
	g->element=(element_type *)malloc(sizeof(element_type)*g->cur_max);   // Simple example for one element, TODO: need to change this COMPLETED
	g->element_countdown = g->element_pause / 20; //1200ms /20ms (20ms braucht ca. ein programmdurchlauf)
	g->cur_act=0;
	g->just_thrown=0;
	g->all_points=0;
	g->toRun=900;

	// Init player position
	p->x=Win_width-Size_tile;
	p->y=(float)CHARACTER_FLOOR;
	int i,j;
	for(i=0; i<9; i++)
		for(j=0; j<10;j++)
			g->comps[i][j]=-1;
}

/***************************************************************************
 * Main game loop: Logic
 ***************************************************************************/
void init_next_element(game_state_type *g, player_data_type *pl)
{
	// TO DO: ...
	// TO DO: Initialize a new competence element, when/before it is thrown by the teacher
	// TO DO: Check if there are further elements or whether the level is completed
	// TO DO: Keep in mind the correct waiting time between elements
	// TO DO: Use the following calculation to initialize x, y, vx, vy (as explained in game rules)
	g->element_countdown--;
	if(g->element_countdown == 0 && g->cur_act!=g->cur_max)//Soll er gerade werfen und sind noch Elemente zum Werfen da
	{
		element_type el;
		//el=(element_type *)malloc(sizeof(element_type));
		g->element_countdown = g->element_pause / 20;
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
		//printf("cur_act: %d\n",g->cur_act);
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
}

// TODO (optional/suggestion):
//        Write a function that checks whether/how two competence elements are colliding
//        Feel free to use the TODO suggestions - or completely ignore them, if you have your own ideas
// return:
// 0 no coll
// 1 side coll
// 2 bridge
// 3 on top
int check_collision(element_type *el1, element_type *el2, game_state_type *g)
{
	// TODO: maybe it helps to distinguish between the current, flying element and the others
	// TODO: overlap between elements?
	// TODO: Reflection on the side?
	// TODO: landing/contact on top?
	// TODO: What about the case, where an element lands on two other elements?
	int i;
	char bridge;
	const float buffer=6.0;
	element_type *el3;

	if(el1->x + Size_comp > el2->x && el1->x + Size_comp - buffer < el2->x &&
			el1->y + Size_comp > el2->y && el1->y < el2->y + Size_comp) {
		el1->vx=(-0.9)*el1->vx;
	}
	else if(el1->x + Size_comp > el2->x && el1->x < el2->x + Size_comp) {
		if(el1->y + Size_comp > el2->y && el1->y + Size_comp < el2->y + buffer) {
			// check if element is not more than half on the element below
			if(el1->x + Size_comp/2 < el2->x || el1->x > el2->x + Size_comp/2) {
				// find another block below (bridge between two blocks)
				i=0;
				bridge=0;
				while(i<=g->cur_act && bridge==0){
					if ((el1->x + Size_comp >= g->element[i].x) &&
							(el1->x <= g->element[i].x + Size_comp) &&
							&(g->element[i])!= el1 && &(g->element[i])!= el2){
						bridge=1;
						el3=&(g->element[i]);
					}
					i++;
				}
				if (!bridge) explode(el1);
				else {
					if(el1->comp!=el2->comp) el1->points+=el2->points;
					if(el1->comp!=el3->comp) el1->points+=el3->points;
					g->all_points+= el1->points;
					return 2;
				}
			}
			else {
				if(el1->comp!=el2->comp) g->all_points+= el2->points;
				return 3;
			}

		}
		else if(el1->y < el2->y + Size_comp + buffer && el1->y > el2->y + Size_comp) {
			explode(el1);
		}
	}


	return 0; // Maybe return some indicator regarding the type of collision?
}

// Update/move the existing competence element(s)
void move_elements(game_state_type *g)
{
	int i,j,coll;
	//printf("ping\n");
	for(i=0;i<g->cur_act;i++) {
		if(g->element[i].comp==4)
		{
			if(g->element[i].countdown!=0)
			{
				g->element[i].countdown--;
			}
			else
			{
				g->element[i].x = Win_width - Size_comp;
				g->element[i].y = Win_height - Size_comp;
				g->element[i].comp = 5;
			}
		}
		else if(g->element[i].y<=Win_floor_y) { // Competence hits floor -> stop moving
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

		}
		else if(g->element[i].x<MIN_PLAYER_X) {
			explode(&g->element[i]);
		}
		else {
			g->element[i].points=1;
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
int init_SDL()
{
	Uint32 color;

    // Initialize SDL 
    if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0 ) {     // SDL_INIT_AUDIO|
        printf("Unable to init SDL: %s\n", SDL_GetError()); exit(1);
    } 
    atexit(SDL_Quit);
    screen = SDL_SetVideoMode(Win_width, Win_height, 32, SDL_DOUBLEBUF | SDL_HWSURFACE);
	if( screen == NULL ) {
        printf("Unable to init video: %s\n", SDL_GetError()); exit(1);
    }	
	// Load graphics
	SDL_WM_SetCaption("Flying Tux: Competence Builder", "Tux_icon.ico");
	graphics = SDL_LoadBMP("competence_builder.bmp");
    if (graphics == NULL) {
	    printf("Unable to load bitmap: %s\n", SDL_GetError());  exit(1);
	}
	// Set transparency color
	color=SDL_MapRGB(screen->format, 255, 200, 200);
	SDL_SetColorKey(graphics, SDL_SRCCOLORKEY | SDL_RLEACCEL, color);
	graphics = SDL_DisplayFormatAlpha(graphics);
	return 0;
}

// Helper function to draw a rectangle with RGB-color
void draw_rect(int x, int y, int w, int h, int r, int g, int b)
{
	SDL_Rect rct;
	Uint32 color;
	
	rct.x=x; rct.y=y; rct.w=w; rct.h=h;
	color=SDL_MapRGB(screen->format, r, g, b);
	SDL_FillRect(screen, &rct, color);
}

// draw graphics for student/teacher/background at position (x, y) (number=0-6) 
void draw_tile(int x, int y, int number)
{
	SDL_Rect src, dest;

	//Set sizes
	src.w = dest.w = Size_tile;
	src.h = dest.h = Size_tile;
	dest.x = x;
	dest.y = y;

	//
	src.x = number*Size_tile;
	src.y = 0;


	SDL_BlitSurface(graphics, &src, screen, &dest);
}

// draw competence elemtents at position (x, y) (number=0-3)
void draw_competence(int x, int y, int number)
{
	SDL_Rect src, dest;

	src.w=src.h=dest.w=dest.h=Size_comp;
	src.x = number*Size_comp;	src.y = Size_tile;
	dest.x = x;	dest.y = y;
	SDL_BlitSurface(graphics, &src, screen, &dest);
}

// Draw digit 'number' (size==1: large, otherwise: small)
void draw_digit(int x, int y, int number, char size)
{
	SDL_Rect src, dest;

	src.w=dest.w=(size? Size_digit_x : Size_smalldigit_x);
	src.h=dest.h=(size? Size_digit_y : Size_smalldigit_y);
	src.x = number*src.w; 
	src.y=Size_tile+Size_comp+(size? 0:Size_digit_y);
	dest.x = x;	dest.y = y;
	SDL_BlitSurface(graphics, &src, screen, &dest);
}

// Draw Global Score
void draw_globalscore(int x, int y, int number)
{
	SDL_Rect src, dest;

	src.x = number*Size_digit_x;
	src.y = Size_comp + Size_tile;
	src.w = dest.w = Size_digit_x;
	src.h = dest.h = Size_digit_y;
	dest.x = x;	dest.y = y;

	SDL_BlitSurface(graphics, &src, screen, &dest);
}

// Draw numbers on blocks
void draw_blockscore(int x, int y, int number)
{
	SDL_Rect src, dest;

	src.y = Size_digit_y + Size_comp + Size_tile;
	src.x = number*Size_smalldigit_x;
	src.w = dest.w = Size_smalldigit_x;
	src.h = dest.h = Size_smalldigit_y;
	dest.x = x;	dest.y = y;

	SDL_BlitSurface(graphics, &src, screen, &dest);
}

/*******************************************************************
 * Refresh the screen and draw all graphics                        *
 *******************************************************************/
void paint_all(game_state_type *g, player_data_type *pl, int key_x)
{
    if(!g)
    	return;
	
    int x, y, i, c, tmpscore, num;
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

	//Draw player
	// TODO: Dynamic position, if jumping
	draw_tile((int)pl->x, CHARACTER_FLOOR, (key_x == 0 ? 0 : 1));

	//Draw teacher
	// TODO (optional): Animate/move the teacher in interesting ways...
	draw_tile(50, CHARACTER_FLOOR, 3 );

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

			}
			//</HackyAsHell>
		}
	}



	// TODO: draw competences, some stupid examples
	//draw_competence(100, 100, 3);
	//draw_competence(400, Win_floor_y, 1);
	//draw_competence(400, 300, 4);







	// TODO (optional): Draw list of remaining competences in curriculum
	
	// TODO (optional): Draw score
	


	SDL_Flip(screen);  // Refresh screen (double buffering)
}


/********************************************************************
 * SDL-Function to check for keyboard events                        *
 ********************************************************************/
int key_control(int *key_x, int *key_c) /* TODO: Final Testing */
{
	SDL_Event keyevent;

	// Verbesserung von Timo: immer +1 / -1, so k�nnen auch kurzzeitig rechts und links gleichzeitig gedr�ckt werden und es ist trotzdem intuitiv

	SDL_PollEvent(&keyevent);
	if(keyevent.type==SDL_KEYDOWN) {
        switch(keyevent.key.keysym.sym){
           case SDLK_LEFT:
        	   (*key_x)--;
        	   break;

           case SDLK_RIGHT:
        	   (*key_x)++;
        	   break;
           case 'a':
        	   auto_control_val = !auto_control_val;
        	   *key_x=0;
        	   break;
           case 's':
        	   *key_c=1;
        	   break;

           case 'f':
        	   *key_c=2;
        	   break;

           case 'p':
        	   *key_c=3;
        	   break;

           case 'r':
        	   *key_c=4;
        	   break;

           case SDLK_ESCAPE:
        	   return 0;
        	   break;

           default: break;
		}
	}

	else if(keyevent.type==SDL_KEYUP) {

		switch(keyevent.key.keysym.sym){

		case SDLK_LEFT:
			(*key_x)++;
			break;

		case SDLK_RIGHT:
			(*key_x)--;
			break;

        case 's':
        case 'f':
        case 'p':
        case 'r':
     	   *key_c=0;
     	   break;

			default: break;
		}
	}
	return 1;
}

/********************************************************************
 * The student-robot                                                *
 * generate left/right player motions automatically                 *
 * TODO: -> this is the big competition challenge!                 *
 *           put your brightest ideas in here to win!               *
 ********************************************************************/
int needed_position(game_state_type *g, player_data_type *pl, int x, int y)//calculates position of the student to place an object perfectly
{
	float studentabstand = (x-Element_start_x)/100;
	float ebene = pow(2,floor(Win_floor_y-y/Size_comp));
	return x;
	/*
	int foundfirst=0;
	float tempx, tempy;
	float tempvx, tempvy;
	tempx=(float)Element_start_x;
	tempy=(float)Win_floor_y;
	tempvx=-(float)sqrt(tempy * 2. * Gravity);
	tempvy=(float)((tempx-tempx)/(-tempvy/Gravity*2.));//nur vor dem Abwurf erlaubt
	float precision = 5.0;
	while (x<tempx+precision && x>tempx-precision && y<tempy+precision && y>tempy+precision)
	{
		tempvy+=Gravity;   // Gravity affects vy
		tempy+=tempvy;     // change position based on speed
		tempx+=tempvx;
	}*/
}

int auto_control(game_state_type *g, player_data_type *pl)
{
	// TODO: This can become the hardest part
	// Generate player motions such that it collects competence
	if(g->just_thrown==1)
	{
		int nextx=-1,nexty=0;
		int nextcomp = g->curriculum[g->cur_act];
		int i,j;
		for(i=9; i>0; i--)//unterste Reihe auslassen
			for(j=9; j>=0;j--)//ganz linke Spalte auslassen
				if(g->comps[i][j]==-1 && g->comps[i-1][j+1]!=nextcomp && g->comps[i-1][j]!=nextcomp && nextx==0 && g->comps[i-1][j]!=-1 && g->comps[i-1][j+1]!=-1)
				{
					nextx=j;
					nexty=i;
				}
		if(nextx==-1)
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
		g->just_thrown=0;
		g->comps[nexty][nextx]=nextcomp;
		int nextxpix=920-(nextx*60);
		int nextypix=Win_floor_y-50-(nextx*60);
		g->toRun = needed_position(g, pl, nextxpix, nextypix);
	}

	int key=0;
	if(g->toRun<pl->x)
		key=-1;
	else if(g->toRun>pl->x)
		key=1;
	else
		key=0;
	return key;

	// The following is a very stupid student example implementation
	// Can you do better?
/*
	static int move_state=0;
	
	move_state=(move_state+1)%50; 
	if(move_state<20) return 1; // move left
	if(move_state>29) return -1; // move right
	return 0; // do not move
	*/
}

// main function
int main(int argc, char *argv[])
{
	// Major game variables: Player + game state
	player_data_type player;
	game_state_type game;
	char pause=0;
	int delay=20;

	// some other variables
	int i, key_x, key_c;
	
	init_SDL();

    load_game_data(&game, "competence_builder.txt");
    init_level(&game, &player);
    game.element_pause=500;
        
    // TODO optional: show_splash_screen, select player, ...
    
    // The main control loop 
    // key_x initialisieren
    key_x = 0;
    key_c = 0;
    // Abbrechen, wenn key_control 0 zur�ckgibt.
	while(key_control(&key_x,&key_c)) {
		if(key_c==1) {
			delay+=10;
		}
		else if(key_c==2) {
			delay-=10;
			if(delay<=20) delay=20;
		}
		else if(key_c==3) {
			pause=1;
		}
		else if(key_c==4) {
			pause=0;
		}
		if(pause==0) {
			//Draw first
			paint_all(&game, &player, key_x);

			init_next_element(&game, &player);
			// TODO: Next level?
			//printf("%d %d\n",game.cur_act,game.element[game.cur_act-1].comp);

			// move_elements is called ten times before the graphics are updated
			// Thus, a better simulation precision is achieved
			// without spending too much performance on (slow) graphics output
			for(i=0; i<10; i++)
				move_elements(&game);

			// Wenn die Automatik l�uft sich dieser auch bedienen
			// Wenn die Automatik l�uft sich dieser auch bedienen
			if (1){//auto_control_val) {
				key_x = 0;
				key_x=auto_control(&game, &player); // use only for 'robot player'
			}

			/* Nur bewegen, wenn
			 * - der Player innerhalb der Bewegungsreichweite ist
			 * - der Player rechts au�erhalb der Bewegungsreichweite ist und sich nach links bewegen m�chte
			 * - der Player links au�erhalb der Bewegungsreichweite ist und sich nach rechts bewegen m�chte
			 */
			if ((player.x + Size_tile <= Win_width && player.x >= MIN_PLAYER_X) || (player.x + Size_tile > Win_width && key_x < 0) || (player.x < MIN_PLAYER_X && key_x > 0)) {
				player.x+=key_x*Player_v_x;    // Calculate new x-position
			}

			paint_all(&game, &player, key_x);  // Update graphics

			SDL_Delay(delay); // wait 20 ms
		}
	}

    free(game.curriculum);
	return 0;
}
