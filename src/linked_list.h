#pragma once

#include <stdbool.h>

typedef struct {
    int x;
    int y;
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
    ListNode* back;
} LinkedList;

bool linkedListContainsTile(LinkedList* list, PathTile tile);
void linkedListRemoveTile(LinkedList* list, PathTile tile);
void linkedListMinPriorityAdd(LinkedList* list, PathTile tile);
void linkedListAddBack(LinkedList* list, PathTile tile);