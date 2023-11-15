#include "list.h"
#include <stdlib.h>
#include <SDL2/SDL_assert.h>
#define ASSERTFUNC SDL_assert


// Private (helper) functions -------------------------------------------------
 
List List_createInitNewList(void){
	List l = malloc( sizeof (struct ListNode) );
	ASSERTFUNC( l && "Null malloc return" );
	l->data = NULL;
	l->next = NULL;
	return l;
}

List List_lastNode( List l ){
	ASSERTFUNC( l && "Null argument" );
	while( l->next ) l = l->next;
	return l;
}


// Exported functions ---------------------------------------------------------
 
List List_addToBack( List list, void* data ){
	// if L is null, empty list is assumed
	// therefore make list the back of the list
	if( !list ) {
		list = List_createInitNewList();
		list->next = NULL;
		list->data = data;
		return list;
	}
	List backOfList = List_lastNode( list );

	// Create new list
	List newNode = List_createInitNewList();

	// Initialize the data in the new list
	newNode->next = NULL;
	newNode->data = data;

	// Hook the back of the list to the new list
	backOfList->next = newNode;

	return list;
}

List List_addToFront( List list, void* data ){
	List newNode = List_createInitNewList();
	newNode->next = list;
	newNode->data = data;
	return newNode;
}

List List_removeFirstNode( List list ){
	ASSERTFUNC( list && "null arguement" );
	List newFirstNode = list->next;
	free( list );
	return newFirstNode;
}

void* List_back( List l ){
	ASSERTFUNC( l && "null arguement" );
	return List_lastNode( l )->data;
}

void* List_front( List l ){
	ASSERTFUNC( l && "null arguement" );
	return l->data;
}

long List_length( List l ){
	ASSERTFUNC( l && "null arguement" );
	long length = 0;
	while( l->next ) {
		l = l->next;
		length++;
	}
	return length;
}
