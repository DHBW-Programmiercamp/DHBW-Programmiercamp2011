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

// Graphics data - SDL variables
SDL_Surface *graphics, *screen;  // Graphics data, screen data

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

// Important data structures, TODO: change/extend if needed

// Competence element
typedef struct {
   float x, y;            // Position
   float vx, vy;          // Speed
   char comp;             // Type of competence 0 - 3,  crash=4
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
} game_state_type;


/*************************************************************************************
 * Initialization
 */

// Load lecture/level plan from file
int load_game_data(game_state_type *g, char *filename)
{
	FILE *file;

    // Open the file for reading
    file=fopen(filename,"r"); 
    if(file==NULL) {
        printf("Error: can't open file.\n");
        return -1;
    }
    
    // TODO: Read level data, allocate memory if needed
    
    fclose(file);    // close file
    return 0;
}

// Initialize next level / player
void init_level(game_state_type *g, player_data_type *p )
{
	g->element=(element_type *)malloc(sizeof(element_type));   // Simple example for one element, TODO: need to change this

	// Init player position
	p->x=Win_width/3.f;
	p->y=(float)Win_floor_y;
}

/***************************************************************************
 * Main game loop: Logic
 ***************************************************************************/
void init_next_element(game_state_type *g, player_data_type *pl)
{
	element_type *el; 

	return; // TODO: ...
	
	// TODO: Initialize a new competence element, when/before it is thrown by the teacher
	
	// TODO: Check if there are further elements or whether the level is completed
	
	// TODO: Keep in mind the correct waiting time between elements

	// TODO: Use the following calculation to initialize x, y, vx, vy (as explained in game rules)
	// The teacher throws in such direction that the competence lands at the current player position
	// For the competition these calculations must be left unchanged! 
	
	el->x=(float)Element_start_x;
	el->y=(float)Win_floor_y; 
	el->vy=-(float)sqrt(el->y * 2. * Gravity);
	el->vx=(float)((pl->x-el->x)/(-el->vy/Gravity*2.));

	// Just in case, some notes regarding the physics - you don't need to understand that
    // h max = v2 / (2g)       v=sqrt(h max * 2 * g)
	// R = v^2 / g * sin (2 beta)
	// R * g = v2 * sin b * cos b
	// Steigzeit: v0/g
}


// TODO (optional/suggestion):
//        Write a function that checks whether/how two competence elements are colliding
//        Feel free to use the TODO suggestions - or completely ignore them, if you have your own ideas
int check_collision(element_type *el1, element_type *el2)
{
	// TODO: maybe it helps to distinguish between the current, flying element and the others
	// TODO: overlap between elements?
	// TODO: Reflection on the side?
	// TODO: landing/contact on top?
	// TODO: What about the case, where an element lands on two other elements?



	if(el1->x + Size_comp > el2->x && el1->x < el2->x + Size_comp) {
		if(el1->y + Size_comp > el2->y && el1->y < el2->y + Size_comp) {
			// side collision

		}
	}
	else if(el2->x + Size_comp > el1->x) {
		if(el1->y < el2->y + Size_comp) {

		}
		else if(el1->y < el2->y + Size_comp) {
			return 1;
		}
	}
	return 0; // Maybe return some indicator regarding the type of collision?
}

// Update/move the existing competence element(s)
void move_elements(game_state_type *g)
{
	int i,j,coll;
	element_type *el;    // Pointer to some competence element
	
	for(i=0;i<g->cur_max;i++) {
		el=&g->element[0];    // Take first element

		if(el->y<Win_floor_y) { // Competence hits floor -> stop moving

			// Check for collision with other elements
			j=-1;
			do {
				j++;
				coll = check_collision(el,g->element[j]);
			} while(j<g->cur_max && coll==0);
			if(coll==0) {
				// Normal motion based on current speed and gravity.
				el->vy+=Gravity;   // Gravity affects vy
				el->y+=el->vy;     // change position based on speed
				el->x+=el->vx;
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

	src.w=dest.w=Size_tile;
	src.h=dest.h=Size_tile;
	src.x=number*Size_tile; src.y=0;
	dest.x=x;	dest.y=y;
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

/*******************************************************************
 * Refresh the screen and draw all graphics                        *
 *******************************************************************/
void paint_all(game_state_type *g, player_data_type *pl)
{
    int x, y;
	
    // Draw background
    draw_rect(0,Win_floor_y,Win_width, Win_height-Win_floor_y,  30,30,50);

	// TODO: draw correct background graphics, e.g. with tables and floor
	for(y=0; y<Win_floor_y; y+=Size_tile)
		for(x=0; x<Win_width/2; x+=Size_tile)
			draw_tile(x, y, 4+(x/Size_tile%3));

	// TODO: draw competences, some stupid examples
	draw_competence(100, 100, 3); 
	draw_competence(400, Win_floor_y, 1); 
	draw_competence(400, 300, 4); 

	// Draw player (better do not change the x,y coordinates)
	draw_tile((int)pl->x-Size_tile/2, (Win_floor_y-1)/Size_tile*Size_tile, 0 /* TODO: Animate player when walking 0<->1 */);

	// Draw teacher 
	// TODO (optional): Animate/move the teacher in interesting ways...
	draw_tile(200, (Win_floor_y-1)/Size_tile*Size_tile, 3 );

	// TODO (optional): Draw list of remaining competences in curriculum
	
	// TODO (optional): Draw score
	
	SDL_Flip(screen);  // Refresh screen (double buffering)
}


/********************************************************************
 * SDL-Function to check for keyboard events                        *
 ********************************************************************/
int key_control()
{
	static int key_x=0;    // Maybe this is of use to you... - otherwise just delete it 
	SDL_Event keyevent;    

	SDL_PollEvent(&keyevent);
	if(keyevent.type==SDL_KEYDOWN) {
        switch(keyevent.key.keysym.sym){
           case SDLK_LEFT: /* TODO: Do something */  break;
           case SDLK_RIGHT: /* TODO: Do something */ break;
	       case SDLK_ESCAPE: return 0; break;
           default: break;
		}
	}
	else if(keyevent.type==SDL_KEYUP) {
		switch(keyevent.key.keysym.sym){
           case SDLK_LEFT: /* TODO: Do something */  break;
           case SDLK_RIGHT: /* TODO: Do something */  break;
           default: break;
		}
	}
	return 1; // Maybe use return value?
}

/********************************************************************
 * The student-robot                                                *
 * generate left/right player motions automatically                 *
 * TODO: -> this is the big competition challenge!                 *
 *           put your brightest ideas in here to win!               *
 ********************************************************************/
int auto_control(game_state_type *g, player_data_type *pl)
{
	// TODO: This can become the hardest part
	// Generate player motions such that it collects competence  
	
	
	// The following is a very stupid student example implementation
	// Can you do better?
	static int move_state=0;
	
	move_state=(move_state+1)%50; 
	if(move_state<20) return 1; // move left
	if(move_state>29) return -1; // move right
	return 0; // do not move
}

// main function
int main(int argc, char *argv[])
{
	// Major game variables: Player + game state
	player_data_type player;
	game_state_type game;

	// some other variables
	int i, key_x;
	
	init_SDL();    

    load_game_data(&game, "competence_builder.txt");
    init_level(&game, &player);
        
    // TODO optional: show_splash_screen, select player, ...
    
    // The main control loop 
    while(key_control()) {
    	
    	init_next_element(&game, &player);
		// TODO: Next level?
    	
    	// move_elements is called ten times before the graphics are updated
    	// Thus, a better simulation precision is achieved 
    	// witout spending too much performance on (slow) graphics output
		for(i=0; i<10; i++)
			move_elements(&game);
		
		// TODO: Check keyboard input for manual player
		// TODO: How to abort the game?
		key_x=auto_control(&game, &player); // use only for 'robot player'

		player.x+=key_x*Player_v_x;    // Calculate new x-position
		// TODO: Check for screen borders / keep your distance from the teacher...
				    	
		paint_all(&game, &player);  // Update graphics
        
		SDL_Delay(20); // wait 20 ms
	}
    
	return 0;
}
