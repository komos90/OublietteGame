#pragma once

#include <stdbool.h>


typedef struct
{
    int x;
    int y;
    int parentX;
    int parentY;
    float heuristic;
} PathTile;

typedef struct ListNode
{
    PathTile tile;
    struct ListNode* next;
} ListNode;

typedef struct
{
    ListNode* front;
} LinkedList;


PathTile* linkedListFindTile      (LinkedList* list, int x, int y);
bool      linkedListContainsTile  (LinkedList* list, PathTile tile);
void      linkedListRemoveTile    (LinkedList* list, PathTile tile);
void      linkedListMinPriorityAdd(LinkedList* list, PathTile tile);
void      linkedListAddFront      (LinkedList* list, PathTile tile);
void      linkedListAddBack       (LinkedList* list, PathTile tile);
void      linkedListRemoveFront   (LinkedList* list);
