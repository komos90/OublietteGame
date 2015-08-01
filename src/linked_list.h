#pragma once

#include <stdbool.h>

typedef struct ListNode_ ListNode;

typedef struct {
    int x;
    int y;
    int parentX;
    int parentY;
    float heuristic;
} PathTile;

typedef struct ListNode_
{
    PathTile tile;
    struct ListNode_* next;
} ListNode;

typedef struct
{
    ListNode* front;
    //ListNode* back;
} LinkedList;

bool linkedListContainsTile(LinkedList* list, PathTile tile);
void linkedListRemoveTile(LinkedList* list, PathTile tile);
void linkedListMinPriorityAdd(LinkedList* list, PathTile tile);
void linkedListAddFront(LinkedList* list, PathTile tile);
void linkedListAddBack(LinkedList* list, PathTile tile);
void linkedListRemoveFront(LinkedList* list);
PathTile* linkedListFindTile(LinkedList* list, int x, int y);