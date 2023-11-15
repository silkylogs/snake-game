#include "snake_logic.h"

#include <stdlib.h>
#include <SDL2/SDL_assert.h>
#define ASSERTFUNC SDL_assert

// SnakeWorld -----------------------------------------------------------------

SnakeWorld SnakeWorld_initNew( int pixelScreenWidth, int pixelScreenHeight ){
	SnakeWorld sw = {0};
	sw.bigPixelScreenWidth  = pixelScreenWidth;
	sw.bigPixelScreenHeight = pixelScreenHeight;
	return sw;
}

// SnakeSegment ---------------------------------------------------------------

void SnakeSegment_moveSegment( SnakeSegment* ss ){
	ASSERTFUNC( ss && "Null input" );
	ss->posX += ss->velX;
	ss->posY += ss->velY;
}

void SnakeSegment_checkAndEnforceBounds( SnakeWorld sw, SnakeSegment* ss ){
	ASSERTFUNC( ss && "Null input" );
	if( ss->posX < 0 ){
		ss->posX = 0;
		ss->velX = 0;
	}
	if( ss->posY < 0 ) {
		ss->posY = 0;
		ss->velY = 0;
	}
	if( ss->posX >= sw.bigPixelScreenWidth ) {
		ss->posX = sw.bigPixelScreenWidth - 1;
		ss->velX = 0;
	}
	if( ss->posY >= sw.bigPixelScreenHeight ) {
		ss->posY = sw.bigPixelScreenHeight - 1;
		ss->velY = 0;
	}
}

// Control suggestion 1:
// allow snake to change direction immediately and bypass Delay() if:
// - inputted direction is perpendicular to the travelling direction
//
// Control suggestion 2:
// Do not allow the snake to travel backwards to travelling velocity if:
// - the length of the snake is more than 1
//
// Control suggestion 3: (helpful on high values passed to Delay())
// Have a queue of inputs recorded during a tick and execute them in
// subsequent ticks
//
// Control suggestion 3.1:
// Have some type of input cancellation intregrated into the above
void SnakeSegment_changeVelocity( SnakeSegment* ss, SnakeSegmentMovementDirection dir ){
	ASSERTFUNC( ss && "Null input" );
	switch( dir ){
	case DIR_UP: {
		ss->velY = -1;
		ss->velX = 0;
		break;
    }

	case DIR_DOWN: {
		ss->velY = 1;
		ss->velX = 0;
		break;
    }
	
	case DIR_LEFT: {
		ss->velX = -1;
		ss->velY = 0;
		break;
    }

	case DIR_RIGHT: {
		ss->velX = 1;
		ss->velY = 0;
		break;
    }

	default: {
		ASSERTFUNC( 0 && "Invalid direction" );
		break;
    }
	}
}

// SnakePlayer ----------------------------------------------------------------

SnakePlayer SnakePlayer_initNewLen( int initialLength ){
	ASSERTFUNC( initialLength > 0 );
	SnakePlayer sp = {0};
	int initPosX = 0;
	int initPosY = 0;
	
	for( int i = 0; i < initialLength; ++i ){
		SnakeSegment* newSegment = calloc( 1, sizeof (SnakeSegment) );
		ASSERTFUNC( newSegment && "Out of memory" );
		newSegment->posX = initPosX;
		newSegment->posY = initPosY++;
		
		sp.segmentList = List_addToBack( sp.segmentList, newSegment );
	}

	return sp;
}

void SnakePlayer_moveAllSegments( SnakePlayer* sp ){
	ASSERTFUNC( sp && "Null input" );

	LIST_ITERATE( node, sp->segmentList ){
		SnakeSegment* ss = (SnakeSegment*)(node->data);
		SnakeSegment_moveSegment( ss );
	}
}

void SnakePlayer_checkAndEnforceBounds( SnakeWorld sw, SnakePlayer* sp ){
	ASSERTFUNC( sp && "Null input" );

	LIST_ITERATE( node, sp->segmentList ){
		SnakeSegment* ss = (SnakeSegment*)(node->data);
		SnakeSegment_checkAndEnforceBounds( sw, ss );
	}
}

bool SnakePlayer_checkOverlap( SnakePlayer sp ){
	bool hasOverlapped = false;

	LIST_ITERATE( node1, sp.segmentList ){
		int numberOfTimesCoordsAreEqual = 0;
		SnakeSegment* ss1 = (SnakeSegment*)(node1->data);
		int keyX = ss1->posX;
		int keyY = ss1->posY;

		LIST_ITERATE( node2, sp.segmentList ){
			SnakeSegment* ss2 = (SnakeSegment*)(node2->data);
			if( (keyX == ss2->posX) && (keyY == ss2->posY) ) numberOfTimesCoordsAreEqual++;
		}

		if( numberOfTimesCoordsAreEqual >= 2 ){
			hasOverlapped = true;
			break;
		}
	}
	
	return hasOverlapped;
}

void SnakePlayer_translateAllSegments( int x, int y, SnakePlayer* sp ){
	ASSERTFUNC( sp && "Null input" );

	LIST_ITERATE( node, sp->segmentList ){
		SnakeSegment* ss = (SnakeSegment*)(node->data);
		ss->posX += x;
		ss->posY += y;
	}
}

void SnakePlayer_incrementLength( SnakePlayer* sp ){
	ASSERTFUNC( sp && "Null input" );

	SnakeSegment* finalSegment = (SnakeSegment*)List_back( sp->segmentList );
	SnakeSegment* newSegment = malloc( sizeof (SnakeSegment) );
	ASSERTFUNC( newSegment && "out of memory" );

	newSegment->velX = finalSegment->velX;
	newSegment->velY = finalSegment->velY;
	newSegment->posX = finalSegment->posX - finalSegment->velX;
	newSegment->posY = finalSegment->posY - finalSegment->velY;

	List_addToBack( sp->segmentList, newSegment );
}

// test functions 
void SnakePlayer_copyVelocityToAllSegments( int newVelX, int newVelY, SnakePlayer* sp ){
	ASSERTFUNC( sp && "Null input" );

	LIST_ITERATE( node, sp->segmentList ){
		SnakeSegment* ss = (SnakeSegment*)(node->data);
		ss->velX = newVelX;
		ss->velY = newVelY;
	}
}

// SegmentTurnLogic -----------------------------------------------------------

// move internal logic to a SnakePlayer function which adds one segment to the back of the player
// make this function a wrapper to call said SnakePlayer function
void SegmentTurnLogic_addTurnPoint( SegmentTurnLogic* stl, SnakeSegment pointDetails ){
	ASSERTFUNC( stl && "null arguement" );

	SnakeSegment* pointDetailCopy = malloc( sizeof (SnakeSegment) );
	ASSERTFUNC( pointDetailCopy && "out of memory" );

	// maybe replace with memcpy?
	pointDetailCopy->velX = pointDetails.velX;
	pointDetailCopy->velY = pointDetails.velY;
	pointDetailCopy->posX = pointDetails.posX;
	pointDetailCopy->posY = pointDetails.posY;

	stl->segmentList = List_addToFront( stl->segmentList, pointDetailCopy );
}

void SegmentTurnLogic_removeTurnPoint( SegmentTurnLogic* stl ){
	ASSERTFUNC( stl && "null arguement" );
	ASSERTFUNC( List_length( stl->segmentList ) > 0 && "Attempting to remove point from empty list" );

	free( List_front( stl->segmentList ) );
	stl->segmentList = List_removeFirstNode( stl->segmentList );
}

// Fruit ----------------------------------------------------------------------

Fruit Fruit_spawnAtRandomCoord( SnakeWorld sw ){
	return (Fruit){
		rand() % sw.bigPixelScreenWidth,
		rand() % sw.bigPixelScreenHeight
	};
}

bool Fruit_touchedPlayerHead( SnakePlayer sp, Fruit f ){
	SnakeSegment* head = (SnakeSegment*)List_front( sp.segmentList );
	return (head->posX == f.posX) && (head->posY == f.posY);
}
