#include "linked_list.h"
#include "monster.h"

#include <stdlib.h>


void linkedListAddBack(LinkedList* list, PathTile tile)
{
    ListNode* listNode = (ListNode*)malloc(sizeof(ListNode));
    listNode->tile = tile; // PROBABLY NEED TO DO THIS MEMBERWISE
    listNode->next = NULL;
    list->back->next = listNode;
    list->back = listNode;
}

bool linkedListContainsTile(LinkedList* list, PathTile tile)
{
    if (list->front == NULL) return false;

    for (ListNode* current = list->front;
         current != NULL;
         current = current->next)
    {
        if (current->tile.x == tile.x &&
            current->tile.y == tile.y) return true;
    }
    return false;
}

void linkedListRemoveTile(LinkedList* list, PathTile tile)
{
    if (list->front == NULL) return;

    ListNode* prev = NULL;
    for (ListNode* current = list->front;
         current != NULL;
         current = current->next)
    {
        if (current->tile.x == tile.x &&
            current->tile.y == tile.y)
        {
            if (prev != NULL)
            {
                prev->next = current->next;
                free(current);
            }
            else {
                list->front = current->next;
                free(current);
            }
        }
        prev = current;
    }
}


void linkedListMinPriorityAdd(LinkedList* list, PathTile tile)
{
    ListNode* listNode = (ListNode*)calloc(1, sizeof(ListNode));
    listNode->tile = tile; // PROBABLY NEED TO DO THIS MEMBERWISE
    if (list->front == NULL) {
        list->front = listNode;
        return;
    } 

    ListNode* prev = NULL;
    ListNode* current = list->front;

    while (current != NULL && current->tile.heuristic < tile.heuristic)
    {
        prev = current;
        current = current->next;
    }

    listNode->next = current;
    if (prev != NULL)
    {
        prev->next = listNode;
    }
    else
    {
        list->front = listNode;
    }
}