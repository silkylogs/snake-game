#ifndef SNAKE_LOGIC_H
#define SNAKE_LOGIC_H

#include "list/list.h"
#include <stdbool.h>

// SnakeWorld -----------------------------------------------------------------

typedef struct SnakeWorld {
	int bigPixelScreenWidth;
	int bigPixelScreenHeight;
} SnakeWorld;


SnakeWorld SnakeWorld_initNew( int pixelScreenWidth, int pixelScreenHeight );

// SnakeSegment ---------------------------------------------------------------

typedef struct SnakeSegment {
	int velX;
	int velY;
	int posX;
	int posY;
} SnakeSegment;

typedef enum SnakeSegmentMovementDirection {
	DIR_UP = 0,
	DIR_DOWN,
	DIR_RIGHT,
	DIR_LEFT,
}SnakeSegmentMovementDirection;

void SnakeSegment_moveSegment( SnakeSegment* ss );
void SnakeSegment_checkAndEnforceBounds( SnakeWorld sw, SnakeSegment* ss );
void SnakeSegment_changeVelocity( SnakeSegment* ss, SnakeSegmentMovementDirection dir );

// SnakePlayer ----------------------------------------------------------------

typedef struct SnakePlayer {
	List segmentList;
} SnakePlayer;

// Creates a new, zero initialized SnakePlayer with an initial length
// New pieces go downward
SnakePlayer SnakePlayer_initNewLen( int initialLength );

// moveSegment but for all pieces
void SnakePlayer_moveAllSegments( SnakePlayer* sp );

// checkAndEnforceBounds but for all pieces
void SnakePlayer_checkAndEnforceBounds( SnakeWorld sw, SnakePlayer* sp );

// aka checkForLoseCondition()
bool SnakePlayer_checkOverlap( SnakePlayer sp );

void SnakePlayer_translateAllSegments( int x, int y, SnakePlayer* sp );

// Increases the length of the snake by one by adding a segment to the back of the snake
void SnakePlayer_incrementLength( SnakePlayer* sp );

// Test functions
// Private asap
void SnakePlayer_copyVelocityToAllSegments( int newVelX, int newVelY, SnakePlayer* sp );

// SegmentTurnLogic -----------------------------------------------------------

typedef struct SnakePlayer SegmentTurnLogic;

// Makes a copy of pointDetails and adds it to the front of the turn list
void SegmentTurnLogic_addTurnPoint( SegmentTurnLogic* stl, SnakeSegment pointDetails );

// Removes the frontmost element of the turn list
void SegmentTurnLogic_removeTurnPoint( SegmentTurnLogic* stl );

// Fruit ----------------------------------------------------------------------

typedef struct Fruit {
	int posX;
	int posY;
} Fruit;

Fruit Fruit_spawnAtRandomCoord( SnakeWorld sw );
bool Fruit_touchedPlayerHead( SnakePlayer sp, Fruit f );

#endif
