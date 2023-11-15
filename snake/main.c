#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

#include "snake_logic/snake_logic.h"
#include "list/list.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

// TODO: make the game into a game
// TODO: make a toggleable debug screen
// TODO: add menu functionality

// Windowing ------------------------------------------------------------------

struct Window {
	int screenWidth ;
	int screenHeight;
	SDL_Window* sdlWindow;
	SDL_Renderer* sdlRenderer;
};

void initSDLComponents( struct Window* gameWindow ){
	// Create window
	gameWindow->sdlWindow = SDL_CreateWindow(
		"Snake",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		gameWindow->screenWidth, gameWindow->screenHeight,
		SDL_WINDOW_SHOWN );
	SDL_assert( gameWindow->sdlWindow && "Unable to create window" );

	// Create renderer for window
	gameWindow->sdlRenderer = SDL_CreateRenderer( gameWindow->sdlWindow, -1, SDL_RENDERER_SOFTWARE );
	SDL_assert( gameWindow->sdlRenderer && "Unable to create renderer" );

	// Initialize render color and mode
	SDL_SetRenderDrawBlendMode( gameWindow->sdlRenderer, SDL_BLENDMODE_BLEND );
	SDL_SetRenderDrawColor( gameWindow->sdlRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE );
	
	// Initialize text rendering
	TTF_Init();
}

void shutdownSDLComponents( struct Window* gameWindow ){
	TTF_Quit();

	SDL_DestroyRenderer( gameWindow->sdlRenderer );
	SDL_DestroyWindow( gameWindow->sdlWindow );
	gameWindow->sdlWindow   = NULL;
	gameWindow->sdlRenderer = NULL;
	SDL_Quit();
}

void setRenderDrawColor( struct Window gameWindow, Uint32 color ){
	// color = rr gg bb aa
	Uint8 r = (Uint8)(color >> 24);
	Uint8 g = (Uint8)(color >> 16);
	Uint8 b = (Uint8)(color >> 8);
	Uint8 a = (Uint8)(color >> 0);
	SDL_SetRenderDrawColor( gameWindow.sdlRenderer, r, g, b, a );
}

// Rendering functions --------------------------------------------------------

void renderDottedGrid( struct Window gameWindow, SnakeWorld sw ){
	SDL_Rect border = { 0, 0, gameWindow.screenWidth, gameWindow.screenHeight };
	SDL_RenderDrawRect( gameWindow.sdlRenderer, &border );

	for( int y = 0; y < sw.bigPixelScreenHeight; ++y ){
		for( int x = 0; x < sw.bigPixelScreenWidth; ++x ){
			int width  = gameWindow.screenWidth  / sw.bigPixelScreenWidth;
			int height = gameWindow.screenHeight / sw.bigPixelScreenHeight;
			SDL_Rect outlineRect = { width*x, height*y, width, height };
			SDL_RenderDrawPoint( gameWindow.sdlRenderer, outlineRect.x, outlineRect.y );
		}
	}
}

void renderLargePixel( struct Window gameWindow, SnakeWorld sw, int pixX, int pixY ){
	int width  = gameWindow.screenWidth  / sw.bigPixelScreenWidth;
	int height = gameWindow.screenHeight / sw.bigPixelScreenHeight;
	SDL_Rect outlineRect = { width*pixX, height*pixY, width, height };
	SDL_RenderFillRect( gameWindow.sdlRenderer, &outlineRect );
}

void renderSnakePlayer( struct Window gameWindow, SnakeWorld sw, SnakePlayer sp ){
	LIST_ITERATE( segment, sp.segmentList ){
		SnakeSegment* ss = (SnakeSegment*)(segment->data);
		renderLargePixel( gameWindow, sw, ss->posX, ss->posY );
	}
}

// Text handling --------------------------------------------------------------

typedef struct TextRect {
	SDL_Texture* texture;
	SDL_Rect rect;
} TextRect;

// x, y: upper left corner.
TextRect createTextRect(
	struct Window w,
	int x, int y,
	char *text, TTF_Font *font
){
	TextRect tr;
	SDL_Color textColor = {135, 125, 100, 100};

	SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(font, text, textColor, w.screenWidth);
	tr.texture = SDL_CreateTextureFromSurface( w.sdlRenderer, surface );

	int text_width = surface->w;
	int text_height = surface->h;
	SDL_FreeSurface(surface);

	tr.rect.x = x;
	tr.rect.y = y;
	tr.rect.w = text_width;
	tr.rect.h = text_height;

	return tr;
}

// Open font for use
TTF_Font* openFont( const char* fontPath, int fontSize ){
	TTF_Font *font = TTF_OpenFont( fontPath, fontSize );
	SDL_assert( font && "Font not found" );
	return font;
}

SDL_Rect positionRectCenter( SDL_Rect oldRect, int xPos, int yPos ){
	SDL_Rect r = oldRect;
	r.x = xPos - r.w/2;
	r.y = yPos - r.h/2;
	return r;
}


// Applies velocity to segments in position
// REFACTOR to meet new data type
void SnakePlayer_applyVelocityToSegmentsInPosition( SnakePlayer* sp, SegmentTurnLogic* stl ){
	SDL_assert(  sp && "SnakePlayer is NULL" );
	SDL_assert( stl && "SegmentTurnLogic is NULL" );

	SnakeSegment* backOfSnakePlayer = List_back( sp->segmentList );
	SnakeSegment* frontOfSegmentTurnList = List_front( stl->segmentList );
	LIST_ITERATE( node, sp->segmentList ){
		SnakeSegment* iteratingSegment = (SnakeSegment*)(node->data);

		// If segment position equals the segment position at the front of the segment turn list
		// set the segment's velocity to be equal to the front of the segment turn list's velocity
		if(
			(iteratingSegment->posX == frontOfSegmentTurnList->posX) &&
			(iteratingSegment->posY == frontOfSegmentTurnList->posY)
		){
			iteratingSegment->velX = frontOfSegmentTurnList->velX;
			iteratingSegment->velY = frontOfSegmentTurnList->velY;
		}
		
		// If the last segment passes through the turning segment
		// remove the frontmost element of the segment turn list
		if( frontOfSegmentTurnList == backOfSnakePlayer ){
			List_removeFirstNode( stl->segmentList ); ///////////////////
		}
	}

}

// I KNOW that main() takes no arguments damnit
#pragma warning(disable:4026)
int main( void ){
	// Initialization -----------------------------------------------------

	struct Window gameWindow  = { 1024, 1024, NULL, NULL };
	SnakePlayer   snakePlayer = {0};
	SnakeWorld    snakeWorld  = {0};
	Fruit         fruit       = {0};

	SegmentTurnLogic segmentTurnList = {0};
	snakeWorld = SnakeWorld_initNew( 31, 31 );
	snakePlayer = SnakePlayer_initNewLen( 2 );
	initSDLComponents( &gameWindow );
	srand((unsigned)time(NULL));

	TTF_Font* font = openFont( "C:/Windows/Fonts/Arial.ttf", 100 );
	TextRect scoreRect = {0};

	// Set initial position and velocity of the player
	SnakeSegment initialSnakeState = {
		.velX = 0,
		.velY = -1,
		.posX = snakeWorld.bigPixelScreenWidth  / 2,
		.posY = snakeWorld.bigPixelScreenHeight / 2
	};
	SnakePlayer_copyVelocityToAllSegments( initialSnakeState.velX, initialSnakeState.velY, &snakePlayer );
	SnakePlayer_translateAllSegments     ( initialSnakeState.posX, initialSnakeState.posY, &snakePlayer );

	
	SnakeSegment turnData = {0};
	// Test code to make the snake turn at (middle of screen, 2)
	// appearently this test code is somehow responsible for making my main program not segfault
	//turnData = (SnakeSegment){
	//	.velX = 1, .velY = 1,
	//	.posX = snakeWorld.bigPixelScreenWidth  / 2, .posY = 2
	//};
	SegmentTurnLogic_addTurnPoint( &segmentTurnList, turnData );
	//

	fruit = Fruit_spawnAtRandomCoord( snakeWorld );
	
	// Game loop ----------------------------------------------------------

	SDL_Event e;
	bool gameRunning = true;
	while( gameRunning ) {

		// Input handling ---------------------------------------------
		
		while( SDL_PollEvent( &e ) != 0 ) {
			if( e.type == SDL_QUIT ) gameRunning = false;
			else if( e.type == SDL_KEYDOWN ){
				turnData = *(SnakeSegment*)List_front( snakePlayer.segmentList );
				SnakeSegment_moveSegment( &turnData );
				switch( e.key.keysym.sym ){

				#define CHANGE_VEL_MACRO( turnData, dir, segmentTurnList )\
					SnakeSegment_changeVelocity( &turnData, dir );\
					SegmentTurnLogic_addTurnPoint( &segmentTurnList, turnData );\
				
				case SDLK_UP:
					CHANGE_VEL_MACRO( turnData, DIR_UP, segmentTurnList )
					break;

				case SDLK_DOWN:
					CHANGE_VEL_MACRO( turnData, DIR_DOWN, segmentTurnList )
					break;
				
				case SDLK_LEFT:
					CHANGE_VEL_MACRO( turnData, DIR_LEFT, segmentTurnList )
					break;

				case SDLK_RIGHT:
					CHANGE_VEL_MACRO( turnData, DIR_RIGHT, segmentTurnList )
					break;
				
				case SDLK_ESCAPE:
					gameRunning = false;
					break;
				}
				#undef CHANGE_VEL_MACRO
			}
		}

		// Delay logic ------------------------------------------------

		int idealDelay = 100;
		SDL_Delay( idealDelay );

		// Gameplay ---------------------------------------------------
		
		SnakePlayer_moveAllSegments( &snakePlayer );
		SnakePlayer_checkAndEnforceBounds( snakeWorld, &snakePlayer );
		if( SnakePlayer_checkOverlap( snakePlayer ) ) gameRunning = false;
		
		if( Fruit_touchedPlayerHead( snakePlayer, fruit ) ){
			SnakePlayer_incrementLength( &snakePlayer );
			fruit = Fruit_spawnAtRandomCoord( snakeWorld );
		}

		SnakePlayer_applyVelocityToSegmentsInPosition( &snakePlayer, &segmentTurnList );

		char scoreText[42] = "Test text";
		_snprintf_s(
			scoreText,
			sizeof scoreText, sizeof scoreText,
			"%d", List_length(snakePlayer.segmentList)+1
		);

		// Render -----------------------------------------------------
		// mayhaps create a defined rendering order for things?

		////// Color         = (Uint32)0xrrggbbaa;
		Uint32 darkAmber     = (Uint32)0x8a5b0cff;
		Uint32 brightAmber   = (Uint32)0xeb9607ff;
		Uint32 pureBlack     = (Uint32)0x000000ff;
		Uint32 fruitColor    = (Uint32)0xdb6817ff;
		//Uint32 turnListColor = (Uint32)0x88000044;

		scoreRect = createTextRect( gameWindow, 0, 0, scoreText, font );

		// Position score text
		scoreRect.rect = positionRectCenter(
			scoreRect.rect,
			gameWindow.screenWidth  / 2,
			gameWindow.screenHeight / 8 );

		setRenderDrawColor( gameWindow, pureBlack );
		SDL_RenderClear( gameWindow.sdlRenderer );

		setRenderDrawColor( gameWindow, darkAmber );
		renderDottedGrid( gameWindow, snakeWorld );

		setRenderDrawColor( gameWindow, brightAmber );
		renderSnakePlayer( gameWindow, snakeWorld, snakePlayer );

		setRenderDrawColor( gameWindow, fruitColor );
		renderLargePixel( gameWindow, snakeWorld, fruit.posX, fruit.posY );

		/* Render the contents of the turn list 
		if( segmentTurnList.segmentList && List_length(segmentTurnList.segmentList) >= 0 ){
			LIST_ITERATE( node, segmentTurnList.segmentList ){
				SnakeSegment* ss = (SnakeSegment*)(node->data);
				setRenderDrawColor( gameWindow, turnListColor );
				renderLargePixel( gameWindow, snakeWorld, ss->posX, ss->posY );
			}
		}*/
	        
		SDL_RenderCopy(gameWindow.sdlRenderer, scoreRect.texture, NULL, &scoreRect.rect);

		SDL_RenderPresent( gameWindow.sdlRenderer );
	}

	// Cleanup ------------------------------------------------------------

	// Free all player segments (and maybe the container, too?)
	// LIST_ITERATE( node, snakePlayer.segmentList ) free( node->data );
	SDL_DestroyTexture( scoreRect.texture );
	TTF_CloseFont(font);
	shutdownSDLComponents( &gameWindow );
	return 0;
}

/*
	//Render red filled quad
	SDL_Rect fillRect = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	SDL_SetRenderDrawColor( sdlRenderer, 0xFF, 0x00, 0x00, 0xFF );		
	SDL_RenderFillRect( sdlRenderer, &fillRect );

	//Render green outlined quad
	SDL_Rect outlineRect = { SCREEN_WIDTH / 6, SCREEN_HEIGHT / 6, SCREEN_WIDTH * 2 / 3, SCREEN_HEIGHT * 2 / 3 };
	SDL_SetRenderDrawColor( sdlRenderer, 0x00, 0xFF, 0x00, 0xFF );		
	SDL_RenderDrawRect( sdlRenderer, &outlineRect );
		
	//Draw blue horizontal line
	SDL_SetRenderDrawColor( sdlRenderer, 0x00, 0x00, 0xFF, 0xFF );		
	SDL_RenderDrawLine( sdlRenderer, 0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2 );

	//Draw vertical line of yellow dots
	SDL_SetRenderDrawColor( sdlRenderer, 0xFF, 0xFF, 0x00, 0xFF );
	for( int i = 0; i < SCREEN_HEIGHT; i += 4 )
	{
		SDL_RenderDrawPoint( sdlRenderer, SCREEN_WIDTH / 2, i );
	}
*/
