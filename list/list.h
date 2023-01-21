#ifndef LINKEDLIST_H
#define LINKEDLIST_H

typedef struct ListNode *List;
struct ListNode {
	struct ListNode* next;
	void* data;
};

// Return value policy:
// each function should return the modified list
// the functions should not return through the arguement, where possible

// The correct iteration syntax through a linked list
#define LIST_ITERATE( iterator, list ) for( List iterator = list; iterator; iterator = iterator->next )

// Adds a void pointer to the back of the list
extern List List_addToBack( List list, void* data );

// Adds a void pointer to the front of the list
extern List List_addToFront( List list, void* data );

// Removes a void pointer from the front of the list without touching the data
extern List List_removeFirstNode( List list );

// Returns the last node in the list
extern void* List_back( List list );

// Returns the first node in the list
extern void* List_front( List list );

// Returns the length of the list
extern long List_length( List list );

#endif // !LINKEDLIST_H

// Usage:
/*
List numList = NULL;
for( int i = 0; i <= 0xfff; ++i ){
	numList = List_addToBack( numList, (void*)i );
}
	
LIST_ITERATE( iterator, numList ){
	printf("%p\n", iterator->data );
}

printf( "Length: %d, Last: %p\n", List_length( numList ), List_back( numList ) );
*/

