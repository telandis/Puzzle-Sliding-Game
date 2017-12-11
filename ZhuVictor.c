//
//  main.c
//  PA1
//
//  Created by Victor Zhu on 12/6/16.
//  Copyright (c) 2016 Telandis. All rights reserved.
//

/*
 Victor Zhu
 COSI 21A Jim Storer
 PA1
 */
#include <stdio.h>
#include <stdlib.h>
#include "Puzzle.h"
#include "Output.h"

static int primevals[] = {12289, 24593, 49157, 786433, 196613, 393241};

typedef struct vbody {//used portions of code from handouts on cosi site
    struct vbody *next; //next board in hash bucket
    struct vbody *prevPuzzPath; //keep track of board this formed from
    struct vbody *nextFinalPath; //at the end create full shortest path
    char data[BoardSize]; //the board itself
    char dataBoard[BoardWidth][BoardHeight];
    int direction;
    int step;//which step it is
    char piece;//which piece moved
} TypeVertex;

typedef TypeVertex *TypeList;//used portions of code from handouts on cosi site

TypeVertex *allocateVertex() {//used portions of code from handouts on cosi site
	TypeVertex *p = NULL;
    p = malloc(sizeof(struct vbody));
	if (p==NULL) {printf("Malloc failed."); exit(1);}
	return p;
}

TypeVertex *constructVbody(char input[BoardSize]) {
    TypeVertex *item = NULL;
    item = allocateVertex();
    item->next = NULL;
    item->prevPuzzPath = NULL;
    item->nextFinalPath = NULL;
    for(int i = 0; i < BoardSize; i++) {
        item->data[i] = input[i];
        int heightindex = i / BoardWidth;
        int widthindex = i % BoardWidth;
        item->dataBoard[widthindex][heightindex] = input[i];
    }
    return item;
}

typedef struct htable {//used portions of code from handouts on cosi site
    TypeVertex *array[HashArraySize];
    int tablesize;
    int amountInBucket[HashArraySize]; // = amountInBucket[HashArraySize]
    int numOfBuckSize[HashStatsMAX];
    //numOfBuckSize initialize to 0s
    //when counting, do for loop iteration of array amountInBucket numOfBuckSize[amountInBucket[i]] += 1
} HashTable;

HashTable *allocateHtable() {//used portions of code from handouts on cosi site
	HashTable *p = NULL;
    p = malloc(sizeof(struct htable));
	if (p==NULL) {printf("Malloc failed."); exit(1);}
	return p;
}

typedef HashTable *TypeHash;//used portions of code from handouts on cosi site

HashTable *constructHtable() {
    HashTable *table = NULL;
    
    table = allocateHtable();
    for (int i = 0; i < HashArraySize; i++) {
        table->array[i] = NULL;
    }
    table->tablesize = HashArraySize;
    return table;
}

int getHashKey(TypeList *L) {
	// generates a hash for a struct, called by search and insert
    TypeVertex *v = *L;
    int hash = 23;
    for (int i = 0; i < BoardSize; i++) {
        int val1 = (int) v->data[i];
        hash = (primevals[i % 6] * hash) + val1;
    }
    if(hash < 0) {
        return (-hash % HashArraySize);
    } else {
        return hash % HashArraySize;
    }
}

void insert(TypeHash *T, TypeList *L) {
	//inserts the struct into the hash table
    HashTable *table = *T;
    TypeVertex *v = *L;
    int hashKey = getHashKey(&v);
    TypeVertex *temp = table->array[hashKey];
    table->array[hashKey] = v;
    table->array[hashKey]->next = temp;
    table->amountInBucket[hashKey] = table->amountInBucket[hashKey] + 1;
}

int search(TypeHash *T, TypeList *L) {
	//takes a struct as input and returns true if element is in
	//HT or False otherwise (1 or 0)
    HashTable *table = *T;
    TypeVertex *v = *L;
    int key = getHashKey(&v);
    TypeVertex *n = table->array[key];
    while (n != NULL) {
        int allninesame = 0;
        for (int i = 0; i < BoardSize; i++) {
            if (n->data[i] == v->data[i]) {
                allninesame++;
            }
        }
        if (allninesame == BoardSize) {
            return 1;
        }
        n = n->next;
    }
    return 0;
}

typedef struct CircleQueue {//used portions of code from handouts on cosi site
    TypeVertex *array[QueueArraySize];
    int front;
    int rear;
    int maxPositionUsed; //largest currQueSize
    int currQueSize; //abs(Qfront-Qrear)
    
} Cqueue;

typedef Cqueue *TypeQueue;//used portions of code from handouts on cosi site

Cqueue *allocateCqueue() {//used portions of code from handouts on cosi site
	Cqueue *p = malloc(sizeof(struct CircleQueue));
	if (p==NULL) {printf("Malloc failed."); exit(1);}
	return p;
}

Cqueue *constructCircleQueue() {
    Cqueue *queue = NULL;
    queue = allocateCqueue();
    for (int i = 0; i < QueueArraySize; i++) {
        queue->array[i] = NULL;
    }
    queue->front = 0;
    queue->rear = 0;
    queue->currQueSize = 0;
    queue->maxPositionUsed = 0;
    return queue;
}

void updateQueue(TypeQueue *queue) {
    Cqueue *x = NULL;
    x = *queue;
    if (&(x->array[x->front]) == NULL) {
        x->currQueSize = 0;
    } else {
        x->currQueSize = abs(x->front - x->rear) + 1;
    }
    if(x->currQueSize > x->maxPositionUsed) {
        x->maxPositionUsed = x->currQueSize;
    }
}

void insertQueue(TypeQueue *queue, TypeList *L) {
    int rear;
    Cqueue *queuepoint = *queue;
    if (queuepoint->front == queuepoint->rear) {
        if (queuepoint->array[queuepoint->rear] == NULL) {
            rear = queuepoint->rear;
        } else {
            rear = (queuepoint->rear + 1) % QueueArraySize;
            
        }
    } else {
        rear = (queuepoint->rear + 1) % QueueArraySize;
    }
    
    queuepoint->rear = rear;
    queuepoint->array[rear] = *L;
    updateQueue(queue);
}

TypeVertex *dequeue(TypeQueue *queue) {
    int front;
    Cqueue *queuepoint = *queue;
    TypeVertex *L = queuepoint->array[queuepoint->front];
    if (queuepoint->front == queuepoint->rear) {
        front = (queuepoint->front + 1) % QueueArraySize;
        queuepoint->rear = front;
    } else {
        front = (queuepoint->front + 1) % QueueArraySize;
    }
    queuepoint->front = front;
    updateQueue(queue);
    return L;
}

char *doOneStep(char *prev, int prevHeight, int prevWidth, int newHeight, int newWidth) {
    int oldindex = (prevHeight*BoardWidth) + prevWidth;
    int newindex =(newHeight*BoardWidth) + newWidth;
    char mover = prev[oldindex];
    char switched = prev[newindex];
    char temp = mover;
    char *new = NULL;
    new = malloc(sizeof(char[BoardSize]));
    for (int i = 0; i < BoardSize; i++) {
        new[i] = prev[i];
    }
    new[oldindex] = switched;
    new[newindex] = temp;
    
    return new;
}

int checkGoal(TypeList *L) {
    TypeVertex *first = *L;
    int allninesame = 0;
    for (int i = 0; i < BoardSize; i++) {
        if (first->data[i] == GoalBoard[i]) {
            allninesame++;
        }
    }
    if (allninesame == BoardSize) {
        return 1;
    }
    return 0;
}

int main() {
    HashTable *table = NULL;
    Cqueue *queue = NULL;
    table = constructHtable();
    queue = constructCircleQueue();
    int puzzlecomplete = 0;//check if puzzle is complete
    TypeVertex *nextpuzzle = NULL;
    nextpuzzle = constructVbody(StartBoard);
    nextpuzzle->step = 0;//initial step number
    
    insert(&table, &nextpuzzle);
    insertQueue(&queue, &nextpuzzle);
    
    TypeVertex *curr = NULL;
    TypeVertex *goalpuzzle = NULL;
    
    while (puzzlecomplete == 0) {
        //dequeue from queue
        curr = dequeue(&queue);
        
        //check all possibilities and add them
        for (int i = 0; i < BoardSize; i++) {
            //if piece is a letter
            int heightindex = i / BoardWidth;
            int widthindex = i % BoardWidth;
            
            TypeVertex *north = NULL;
            TypeVertex *east = NULL;
            TypeVertex *south = NULL;
            TypeVertex *west = NULL;
            char currletter = curr->dataBoard[widthindex][heightindex];
            if (currletter != SymbolBlank && currletter != SymbolFixed) {
                //find all puzzles from moving this piece
                
                //create north
                if (heightindex-1 >= 0) {
                    //check if moving north is possible
                    if (curr->dataBoard[widthindex][heightindex-1] == SymbolBlank) {
                        north = constructVbody(doOneStep(curr->data, heightindex, widthindex, heightindex-1, widthindex));
                        north->direction = 0;
                        north->prevPuzzPath = curr;
                        north->step = (curr->step)+1;
                        north->piece = currletter;
                        }
                }
                
                //create east
                if (widthindex+1 < BoardWidth) {
                    //check if moving east is possible
                    if (curr->dataBoard[widthindex+1][heightindex] == SymbolBlank) {
                        east = constructVbody(doOneStep(curr->data, heightindex, widthindex, heightindex, widthindex+1));
                        east->direction = 1;
                        east->prevPuzzPath = curr;
                        east->step = (curr->step)+1;
                        east->piece = currletter;
                        }
                }
                
                //create south
                if (heightindex+1 < BoardHeight) {
                    //check if moving north is possible
                    if (curr->dataBoard[widthindex][heightindex+1] == SymbolBlank) {
                        south = constructVbody(doOneStep(curr->data, heightindex, widthindex, heightindex+1, widthindex));
                        south->direction = 2;
                        south->prevPuzzPath = curr;
                        south->step = (curr->step)+1;
                        south->piece = currletter;
                        }
                }
                
                //create west
                if (widthindex-1 >= 0) {
                    //check if moving east is possible
                    if (curr->dataBoard[widthindex-1][heightindex] == SymbolBlank) {
                        west = constructVbody(doOneStep(curr->data, heightindex, widthindex, heightindex, widthindex-1));
                        west->direction = 1;
                        west->prevPuzzPath = curr;
                        west->step = (curr->step)+1;
                        west->piece = currletter;
                    }
                }
                //check if puzzle is in hashtable
                //add puzzle to hashtable
                //add puzzle to queue
                
                if (north != NULL) {
                    if (checkGoal(&north) == 1) {
                        goalpuzzle = north;
                        puzzlecomplete = 1;
                        break;
                    }
                }
                if (east != NULL) {
                    if (checkGoal(&east) == 1) {
                        goalpuzzle = east;
                        puzzlecomplete = 1;
                        break;
                    }
                }
                if (south != NULL) {
                    if (checkGoal(&south) == 1) {
                        goalpuzzle = south;
                        puzzlecomplete = 1;
                        break;
                    }
                }
                if (west != NULL) {
                    if (checkGoal(&west) == 1) {
                        goalpuzzle = west;
                        puzzlecomplete = 1;
                        break;
                    }
                }
                if (puzzlecomplete == 0) { // game not over, add all to hash table and queue
                    //check if puzzle is in hashtable
                    if (north != NULL) {
                        if (search(&table, &north) == 0) {
                            //add puzzle to hashtable and queue
                            insert(&table, &north);
                            insertQueue(&queue, &north);
                        } else {
                            free(north);
                        }
                    }

                    //check if puzzle is in hashtable
                    if (east != NULL) {
                        if (search(&table, &east) == 0) {
                            //add puzzle to hashtable and queue
                            insert(&table, &east);
                            insertQueue(&queue, &east);
                        } else {
                            free(east);
                        }
                    }
                    
                    //check if puzzle is in hashtable
                    if (south != NULL) {
                        if (search(&table, &south) == 0) {
                            //add puzzle to hashtable and queue
                            insert(&table, &south);
                            insertQueue(&queue, &south);
                        } else {
                            free(south);
                        }
                    }
                    
                    //check if puzzle is in hashtable
                    if (west != NULL) {
                        if (search(&table, &west) == 0) {
                            //add puzzle to hashtable and queue
                            insert(&table, &west);
                            insertQueue(&queue, &west);
                        } else {
                            free(west);
                        }
                    }
                }
                //check if any are solution
            }
            //else check next piece
        }
        
        
    }
    free(table);
    /*Print outputs.*/
    //use goalpuzzle
    TypeVertex *traverse = NULL;
    TypeVertex *temp = NULL;
    traverse = goalpuzzle;
    
    while (traverse->prevPuzzPath != NULL) {
        temp = traverse;
        traverse = traverse->prevPuzzPath;
        traverse->nextFinalPath = temp;
    }
    
    while (traverse != NULL) {
        OutputPosition(traverse->data, traverse->step, traverse->piece, traverse->direction);
        traverse = traverse->nextFinalPath;
    }
    
    /*Print queue statistics.*/
    QueueStats(QueueArraySize, queue->maxPositionUsed, queue->front, queue->rear);
    
    /*Print hash statistics.*/
    for (int i = 0; i < HashArraySize; i++) {
        table->numOfBuckSize[table->amountInBucket[i]] += 1;
    }
    int check = HashStatsMAX - 1;
    int maxBuck = 0;
    int hashSize = 0;
    for (int i = check; i >= 0; i--) {
        if (table->numOfBuckSize[i] > 0) {
            if (i > maxBuck) {
                maxBuck = i;
            }
        }
    }
    for (int i = 0; i < HashArraySize; i++) {
        hashSize += table->amountInBucket[i];
    }
    HashStats(HashArraySize, hashSize, maxBuck);
    
    /* print bucket stats */
    for(int i = 0; i < HashStatsMAX; i++) {
        if (table->numOfBuckSize[i] > 0) {
            BucketStat(i, table->numOfBuckSize[i]);
        }
    }
    return 0;
}