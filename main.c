//
//  main.c
//  BPlusTree
//  Created by Melina Mast on 20.03.16. based on implementation of http://www.amittai.com/prose/bpt.c and Database Systems:The Complete Book

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <stddef.h>
#include "timeset.c"

/******************************TIMESTAMP DEF********************************************/


#define TIMESTAMP_DIFF 300
#define DEBUG_PRINT

//t = type
typedef unsigned long timeStampT;

/******************************STRUCTS - CIRCULAR ARRAY********************************************/


typedef struct Serie{
    timeStampT time;
    double value;
}Serie;

typedef struct CircularArray{
    Serie * data;
    int size;
    int lastUpdatePosition;
    int count;
}CircularArray;

CircularArray *CircularArray_new(int size) {
    CircularArray *array = NULL;
    array = malloc(sizeof(CircularArray));
    array->size = size;
    array->lastUpdatePosition = 0;
    array->data = malloc(array->size * sizeof(Serie));
    array->count = 0;
    return array;
}

void CircularArray_destroy(CircularArray *array) {
    free(array->data);
    free(array);
}


/******************************STRUCTS - DOUBLY LINKED LIST AND LEAF KEYS********************************************/

typedef struct ListValue{
    timeStampT timestamp;
    struct ListValue *prev, *next;
}ListValue;


ListValue *listValue_new(timeStampT time) {
    ListValue *newListValue = malloc(sizeof(ListValue));
    newListValue->timestamp = time;
    
    //link to same value
    newListValue->prev = newListValue;
    newListValue->next = newListValue;
    
    return newListValue;
}



void list_destroy(ListValue *head) {
    
    ListValue * tail = head->prev;
    ListValue * next = head;
    while(head != tail)
    {
        next = head;
        head = head->next;
        free(next);
        
    }
    
    free(head);
}

/******************************STRUCTS - BPLUSTREE*******************************************************************/


typedef struct Node{
    struct Node *parent;
    void ** pointers;
    int numOfKeys;
    //can contain records or innerNodes
    double * keys;
    bool isLeaf;
    //for leafs
    struct Node *prev, *next;
}Node;



Node * Node_new(int nodeSize){
    Node * node = malloc(sizeof(Node));
    node->keys = malloc(nodeSize * sizeof(double));
    node->numOfKeys = 0;
    //always n+1 pointers
    node->isLeaf = false;
    node->pointers = malloc((nodeSize + 1) * sizeof(void *));
    node->next = NULL;
    node->prev = NULL;
    return node;
}

Node *Leaf_new(int nodeSize){
    Node * node = malloc(sizeof(Node));
    node->keys = malloc(nodeSize * sizeof(double));
    node->numOfKeys = 0;
    //always n pointers
    node->isLeaf = true;
    node->pointers = malloc((nodeSize) * sizeof(void *));
    node->next = NULL;
    node->prev = NULL;
    
    return node;
}

void Node_destroy(Node *node) {
    
    if(node->isLeaf){
        for(int i = 0; i < node->numOfKeys; i++){
            list_destroy(node->pointers[i]);
        }
        
    }
    
    free(node->pointers);
    free(node->keys);
    
}

typedef struct BPlusTree{
    struct Node *root;
    int nodeSize;
}BPlusTree;

BPlusTree *BPlusTree_new(int nodeSize) {
    BPlusTree *bPlus = malloc(sizeof(BPlusTree));
    bPlus->nodeSize = nodeSize;
    bPlus->root = NULL;
    return bPlus;
}

void destroyTreeNodes(Node * node) {
    int i;
    
    if (node->isLeaf){
        for (i = 0; i < node->numOfKeys; i++){
            list_destroy(node->pointers[i]);
        }
        
    }else{
        for (i = 0; i < node->numOfKeys + 1; i++){
            destroyTreeNodes(node->pointers[i]);
        }
        
    }
    
    free(node->pointers);
    free(node->keys);
    free(node);
}


void BPlusTree_destroy(BPlusTree *tree) {
    destroyTreeNodes(tree->root);
    free(tree);
}

typedef struct NeighborhoodPosition{
    ListValue * timeStampPosition;
    Node * LeafPosition;
    int indexPosition;
}NeighborhoodPosition;


typedef struct Neighborhood{
    int patternLength;
    int offset;
    double key;
    NeighborhoodPosition leftPosition;
    NeighborhoodPosition rightPosition;
}Neighborhood;



typedef struct {
    timeStampT timestamp;
    double value;
} Measurement;




/*************************************** MASTER METHOD *******************************************/

void shift(BPlusTree *tree, CircularArray *array, timeStampT time, double value);

/*************************************** CIRCULAR ARRAY *******************************************/
bool lookup(CircularArray *array, timeStampT t, double *value);
void printArray(CircularArray * array, int arraySize);
void serie_update(BPlusTree *tree, CircularArray *array, timeStampT t, double value);

/*************************************** NEIGHBOUR *******************************************/
timeStampT neighbor(BPlusTree *tree, double value, TimeSet * set);

void initialize_tree(BPlusTree *tree);

void addRecordToTree(BPlusTree *tree, timeStampT t, double value);
void newTree(BPlusTree *tree, timeStampT time, double value);
void insertRecordIntoLeaf(BPlusTree *tree, Node *leaf, timeStampT t, double firstValue);
int getSplitPoint(int length);
void insertIntoParent(BPlusTree *tree, Node *oldChild, double newKey, Node *newChild);
void insertIntoANewRoot(BPlusTree *tree, Node * left, double value, Node * right);
void splitAndInsertIntoInnerNode(BPlusTree *tree, Node * parent, int leftIndex, double key, Node * rightNode);
int getLeftPointerPosition(Node * parent, Node * left);
void splitAndInsertIntoLeaves(BPlusTree *tree, Node *oldNode, timeStampT time, double firstValue);
void insertIntoTheNode(Node * node, int index, double value, Node * newChild);

Node * findLeaf(BPlusTree *tree, double newKey);


/******** DELETION ********/
bool isDuplicateKey(Node * curNode, timeStampT newTime, double newKey);
void deleteFirstListValue(Node * leaf, int index);
int findLeafKeyIndexAndSetboolIfMultipleListValues(Node * node, double key, bool *hasMultipleTimes);
void delete(BPlusTree *tree, timeStampT time, double value);
void deleteEntry(BPlusTree *tree, Node *node, double toDelete, Node * pointer);
Node * removeEntryFromTheNode(BPlusTree *tree, Node * node, double toDelete, Node * pointerNode);
void adjustTheRoot(BPlusTree *tree);

void mergeNodes(BPlusTree *tree, Node *node, Node *neighbor, int neighborIndex, double kPrime);
void redestributeNodes(BPlusTree *tree, Node *node, Node *neighbor, int neighborIndex, int kIndex, double pointer);

int getNeighbourIndex(Node * node);
void printTree(BPlusTree *tree);

/************** DUPLICATE HANDLING ****************/
void addDuplicateToDoublyLinkedList(Node *node, timeStampT newTime, double duplicateKey);
int getInsertPoint(BPlusTree *tree, Node *oldNode, double leafKey);

/************** PRINT ****************/
void printLevelOrder(Node* root);
void addAndDeleteSomeExampleValuesFromTree(BPlusTree * tree);
void exampleShifts(BPlusTree * tree, CircularArray * array);
void random_shifts(BPlusTree * tree, CircularArray * array);


/************** NEIGHBORHOOD ****************/

Neighborhood *Neighborhood_new(BPlusTree *tree, Measurement *measurement,int patternLength, int offset);
void Neighborhood_destroy(Neighborhood *self);
ListValue * getTMinus(Neighborhood *self, ListValue * currentMinusListValue, Node *currentLeftLeaf, bool isLeftMostKey, int steps);
ListValue * getTPlus(Neighborhood *self, ListValue * currentPlusListValue, Node *currentPlusLeaf, bool isRightMostKey, int steps);
bool Neighborhood_grow(Neighborhood *self, TimeSet *timeset, timeStampT *timestamp);


Measurement * Measurement_new(timeStampT time, double value){
    Measurement * measurement = malloc(sizeof(Measurement));
    measurement->timestamp = time;
    measurement->value = value;
    
    return measurement;
}

void Measurement_destroy(Measurement *measurement){
    
    free(measurement);
    
}

/*
 * Initializes a new neighborhood in the B+ tree.
 *
 * Parameters:
 *   tree: The  SBTree for this neighborhood
 *   Serie: Serie that constitutes this pattern cell
 *   pattern_length: length of the query pattern
 *   offset: position of the Serie within the query pattern
 *           cell. offset=1 means the oldest time point in the
 *           query pattern, offset=pattern_length means the latest
 *           time point in the query pattern.
 */
Neighborhood *Neighborhood_new(BPlusTree *tree, Measurement *measurement,int patternLength, int offset){
    
    Neighborhood *newNeighborhood = malloc(sizeof(Neighborhood));
    NeighborhoodPosition leftNeighbourhoodPos;
    NeighborhoodPosition rightNeighbourhoodPos;
    
    newNeighborhood->key = measurement->value;
    newNeighborhood->offset = offset;
    newNeighborhood->patternLength = patternLength;
    
    bool hasMultipleTimes = false;
    Node * leafNode;
    
    leafNode = findLeaf(tree, measurement->value);
    int pointerIndex = findLeafKeyIndexAndSetboolIfMultipleListValues(leafNode, measurement->value, &hasMultipleTimes);
    
    ListValue *listValueOnThatKey = NULL;
    listValueOnThatKey = leafNode->pointers[pointerIndex];
    
    //The value will be at most patternLength away
    int maxSteps = patternLength;
    while(listValueOnThatKey->timestamp != measurement->timestamp && maxSteps != 0){
        //go from newest value back towards oldest
        listValueOnThatKey = listValueOnThatKey->prev;
        maxSteps--;
    }
    
    leftNeighbourhoodPos.indexPosition = pointerIndex;
    leftNeighbourhoodPos.LeafPosition = leafNode;
    leftNeighbourhoodPos.timeStampPosition = listValueOnThatKey;
    
    rightNeighbourhoodPos.indexPosition = pointerIndex;
    rightNeighbourhoodPos.LeafPosition = leafNode;
    rightNeighbourhoodPos.timeStampPosition = listValueOnThatKey;
    
    newNeighborhood->leftPosition = leftNeighbourhoodPos;
    newNeighborhood->rightPosition = rightNeighbourhoodPos;
    
    return newNeighborhood;
    
    
}

/*
 * Destroys the Neighborhood and frees all allocated memory
 * Parameters:
 *   self: the Neighborhood
 */
void Neighborhood_destroy(Neighborhood *self){
    free(self);
}

bool isLeftMost(NeighborhoodPosition *pos){
    bool leftMostKey = false;
    
    //left leaf or right leaf must be checked
    if(0 == pos->indexPosition){
        leftMostKey = true;
    }
    return leftMostKey;
}

bool isRightMost(NeighborhoodPosition *pos){
    bool rightMostKey = false;
    
    if((pos->LeafPosition->numOfKeys - 1) == pos->indexPosition){
        rightMostKey = true;
    }
    
    return rightMostKey;
}

NeighborhoodPosition getTMinusNeighborHoodPosition(NeighborhoodPosition currentTMinusPos){
    
    bool isLeftMostKey = isLeftMost(&currentTMinusPos);
    
    int currentIndex = currentTMinusPos.indexPosition;
    
    //sibling is in doubly linked list
    if(currentTMinusPos.timeStampPosition->prev->timestamp < currentTMinusPos.timeStampPosition->timestamp){
        
        currentTMinusPos.timeStampPosition = currentTMinusPos.timeStampPosition->prev;
    }
    
    //needs to change key
    else{
        
        //same node
        if(!isLeftMostKey){
            
            int newIndexPostion = currentIndex -1;
            currentTMinusPos.indexPosition = newIndexPostion;
            currentTMinusPos.timeStampPosition = ((ListValue *)currentTMinusPos.LeafPosition->pointers[newIndexPostion])->prev;
        }
        //left neighbour
        else{
            
            if(currentTMinusPos.LeafPosition->prev != NULL){
                
                Node *neighbourNode = currentTMinusPos.LeafPosition->prev;
                ListValue * sibling = neighbourNode->pointers[neighbourNode->numOfKeys - 1];
                
                //get the newest value in doubly linked list
                currentTMinusPos.LeafPosition = currentTMinusPos.LeafPosition->prev;
                currentTMinusPos.timeStampPosition = sibling->prev;
                currentTMinusPos.indexPosition = neighbourNode->numOfKeys-1;
            }
            //is null
            else{
                
                currentTMinusPos.timeStampPosition = NULL;
            }
        }
    }
    
    return currentTMinusPos;
}

NeighborhoodPosition getTPlusNeighborHoodPosition(NeighborhoodPosition currentTPlusPosition){
    
    bool isRightMostKey = isRightMost(&currentTPlusPosition);
    
    int currentIndex = currentTPlusPosition.indexPosition;
    
    if(currentTPlusPosition.timeStampPosition->next->timestamp > currentTPlusPosition.timeStampPosition->timestamp){
        //update listValue Position
        currentTPlusPosition.timeStampPosition = currentTPlusPosition.timeStampPosition->prev;
    }
    else{
        if(!isRightMostKey){
            
            int newIndexPostion = currentIndex + 1;
            currentTPlusPosition.indexPosition = newIndexPostion;
            currentTPlusPosition.timeStampPosition = ((ListValue *)currentTPlusPosition.LeafPosition->pointers[newIndexPostion])->next;
            
        }
        else{
            
            if(currentTPlusPosition.LeafPosition->next != NULL){
                
                Node *neighbour = currentTPlusPosition.LeafPosition->next;
                //newest value uf doubly linked list from left neighbour
                ListValue *oldestSibling = neighbour->pointers[0];
                
                currentTPlusPosition.LeafPosition = currentTPlusPosition.LeafPosition->next;
                currentTPlusPosition.timeStampPosition = oldestSibling;
                currentTPlusPosition.indexPosition = 0;
                
            }
            else{
                currentTPlusPosition.timeStampPosition = NULL;
            }
        }
    }
    
    return currentTPlusPosition;
    
}


void print_Neighborhood(Neighborhood * neighborhood){
    
    printf("key %fl", neighborhood->key);
    printf("\nleft timestamp %fl", (double)neighborhood->leftPosition.timeStampPosition->timestamp);
    printf("\nright timestamp %fl\n",(double)neighborhood->rightPosition.timeStampPosition->timestamp);
}

timeStampT getOffsetTime(Neighborhood * self, NeighborhoodPosition neighborhoodPosition){
    
    timeStampT offsetTime = neighborhoodPosition.timeStampPosition->timestamp + ((self->patternLength  - self->offset) * TIMESTAMP_DIFF);
    
    return offsetTime;
}


/*
 * Grows the neighborhood by one new value and returns its time point via the timestamp
 * parameter. The function returns true if there was a new unseen value and false otherwise
 *
 * Parameters
 *   self: the neighborhood
 *   timeset: a set of seen time points
 *   timestamp: used as a return value, contains the time point of the
 *              new still unseen value discovered by this function
 */

bool Neighborhood_grow(Neighborhood *self, TimeSet *timeset, timeStampT *timestamp){
    
    NeighborhoodPosition leftNeighborhoodPosition = self->leftPosition;
    NeighborhoodPosition rightNeighborhoodPosition = self->rightPosition;
    
    
    bool neighborHoodHasGrown = true;
    
    timeStampT offsetMinusTime = -1;
    timeStampT offsetPlusTime = -1;
    
    leftNeighborhoodPosition = getTMinusNeighborHoodPosition(leftNeighborhoodPosition);
    if(leftNeighborhoodPosition.timeStampPosition != NULL){
        offsetMinusTime = getOffsetTime(self, leftNeighborhoodPosition);
    }
    
    rightNeighborhoodPosition = getTPlusNeighborHoodPosition(rightNeighborhoodPosition);
    if(rightNeighborhoodPosition.timeStampPosition != NULL){
        offsetPlusTime = getOffsetTime(self, rightNeighborhoodPosition);
    }
    
#ifdef DEBUG_PRINT
    
    if(leftNeighborhoodPosition.timeStampPosition != NULL && rightNeighborhoodPosition.timeStampPosition != NULL){
        printf("\ntminus %fl", (double) leftNeighborhoodPosition.timeStampPosition->timestamp);
        printf("\ntplus %fl", (double) rightNeighborhoodPosition.timeStampPosition->timestamp);
        
        printf("\noffsetTMinus %fl", (double) offsetMinusTime);
        printf("\noffsetPlusTime %fl", (double) offsetPlusTime);
    }
    
#endif
    
    //checks if offsetTime is in timestamp set
    while(leftNeighborhoodPosition.timeStampPosition != NULL && TimeSet_contains(timeset, offsetMinusTime)){
        leftNeighborhoodPosition = getTMinusNeighborHoodPosition(leftNeighborhoodPosition);
        offsetMinusTime = getOffsetTime(self, leftNeighborhoodPosition);
        
        //next time doesnt have to check this position again
        self->leftPosition = leftNeighborhoodPosition;
    }
    
    while(rightNeighborhoodPosition.timeStampPosition != NULL && TimeSet_contains(timeset, offsetPlusTime)){
        rightNeighborhoodPosition = getTPlusNeighborHoodPosition(rightNeighborhoodPosition);
        offsetPlusTime = getOffsetTime(self, rightNeighborhoodPosition);
        
        //next time doesnt have to check this position again
        self->rightPosition = rightNeighborhoodPosition;
    }
    
    
    
    
    if(leftNeighborhoodPosition.timeStampPosition != NULL && rightNeighborhoodPosition.timeStampPosition != NULL){
        
        long tMinusdifference = fabs(leftNeighborhoodPosition.LeafPosition->keys[leftNeighborhoodPosition.indexPosition] - self->key);
        long tPlusdifference = fabs(rightNeighborhoodPosition.LeafPosition->keys[rightNeighborhoodPosition.indexPosition] - self->key);
        
        
        if(tMinusdifference <= tPlusdifference){
            
            self->leftPosition = leftNeighborhoodPosition;
            *timestamp = leftNeighborhoodPosition.timeStampPosition->timestamp;
        }
        else{
            self->rightPosition = rightNeighborhoodPosition;
            *timestamp = rightNeighborhoodPosition.timeStampPosition->timestamp;
            
        }
    }
    else if(leftNeighborhoodPosition.timeStampPosition != NULL){
        self->leftPosition = leftNeighborhoodPosition;
        
        *timestamp = leftNeighborhoodPosition.timeStampPosition->timestamp;
        
    }
    else if(rightNeighborhoodPosition.timeStampPosition != NULL){
        self->rightPosition= rightNeighborhoodPosition;
        
        *timestamp = rightNeighborhoodPosition.timeStampPosition->timestamp;
    }
    else{
        neighborHoodHasGrown = false;
    }
    
#ifdef DEBUG_PRINT
    printf("\ntfound %fl\n", (double) *timestamp);
    
#endif
    //offset timestamp will be added later
    
    return neighborHoodHasGrown;
    
}


/****************************** METHODS *******************************************************************/

int main(int argc, const char * argv[]) {
    
    //variable - can be commandLine Input
    int arraySize = 20;
    
    int treeNodeSize = 4;
    
    CircularArray * array = CircularArray_new(arraySize);
    
    //create BplusTree;
    BPlusTree * tree = BPlusTree_new(treeNodeSize);
    exampleShifts(tree, array);
    
    CircularArray * array2 = CircularArray_new(arraySize);
    
    //create BplusTree;
    BPlusTree * tree2 = BPlusTree_new(treeNodeSize);
    random_shifts(tree2, array2);
    
    Measurement * newMeasurement = Measurement_new(3900, 50);
    int patternLength = 4;
    int offset = 2;
    Neighborhood *newNeighborhood = Neighborhood_new(tree, newMeasurement, patternLength, offset);
    
    Measurement_destroy(newMeasurement);
    timeStampT foundTimestamp = -1;
    
    TimeSet *timeSet = TimeSet_new();
    TimeSet_add(timeSet, 3900);
    
    
    Neighborhood_grow(newNeighborhood, timeSet, &foundTimestamp);
    Neighborhood_grow(newNeighborhood, timeSet, &foundTimestamp);
    Neighborhood_grow(newNeighborhood, timeSet, &foundTimestamp);
    
    double value = -1;
    bool x = false;
    
    x = lookup(array, 3900, &value);
    
#ifdef DEBUG_PRINT
    
    printf("bool: %d\n", x);
    printf("value: %fl\n", value);
    
    
    print_Neighborhood(newNeighborhood);
    
#endif
    
    TimeSet_destroy(&timeSet);
    BPlusTree_destroy(tree);
    Neighborhood_destroy(newNeighborhood);
    CircularArray_destroy(array);
    BPlusTree_destroy(tree2);
    CircularArray_destroy(array2);
    
    return 0;
}

void shift(BPlusTree *tree, CircularArray *array, timeStampT time, double value){
    
    //just new values are added to the tree and to the circular array
    serie_update(tree, array, time, value);
    
}

/****************************** PRINT TREE *******************************************************************/

void random_shifts(BPlusTree * tree, CircularArray * array){
    srand(time(NULL));
    
    timeStampT time = 0;
    for (int i = 0; i < 500; ++i) {
        time += TIMESTAMP_DIFF;
        double rand_value = rand() % 50;
        shift(tree, array, time, rand_value);
    }
#ifdef DEBUG_PRINT
    printf("\n \n");
    
    
    printLevelOrder(tree->root);
#endif
    
}

void exampleShifts(BPlusTree * tree, CircularArray * array){
    
    shift(tree, array, 300, 300);
    shift(tree, array, 600, 200);
    shift(tree, array, 900, 600);
    
    shift(tree, array, 1200, 600);
    shift(tree, array, 1500, 1200);
    
    shift(tree, array, 1800, 1500);
    //arrive late
    shift(tree, array, 2100, 300);
    shift(tree, array, 2400, 900);
    shift(tree, array, 2700, 2100);
    
    
    shift(tree, array, 3000, 0);
    shift(tree, array, 3300, 10);
    shift(tree, array, 3600, 20);
    shift(tree, array, 3900, 50);
    
    shift(tree, array, 4200, 50);
    
    
    shift(tree, array, 4500, 60);
    
#ifdef DEBUG_PRINT
    printf("\n \n");
    
    
    printLevelOrder(tree->root);
    
#endif
    
}

int height(Node * node){
    if (node == NULL || node->isLeaf){
        return 1;
    }
    else
    {
        //compute the height of each subtree
        int length = height(node->pointers[0]);
        return length + 1;
    }
}

// Print nodes at a given level
void printGivenLevel(Node * root, int level){
    if (root == NULL){
    }
    else if (level == 1){
        
        for(int i = 0; i < root->numOfKeys; i++){
            printf(" %fl ", root->keys[i]);
        }
        printf("\t \t");
        
        
    }
    
    else if (level > 1)
    {
        for(int i = 0; i<root->numOfKeys+1; i++){
            printGivenLevel(root->pointers[i], level-1);
        }
        
    }
    
}

// Function to print level order traversal a tree
void printLevelOrder(Node * root){
    int h = height(root);
    int i;
    printf("\t \t \t ");
    for (i=1; i<=h; i++){
        printGivenLevel(root, i);
        
        printf("\n");
    }
}


/****************************** DELETION *******************************************************************/

void deleteEntry(BPlusTree *tree, Node *node, double toDelete, Node * pointer){
    
    int minNumberofKeys;
    Node * neighbor;
    int neighbourIndex;
    int kIndex;
    int capacity;
    double innerKeyPrime;
    
    // Remove key and pointer from node.
    node = removeEntryFromTheNode(tree, node, toDelete, pointer);
    
    if (node == tree->root){
        adjustTheRoot(tree);
        return;
    }
    
    // deletion from a innernode or leaf-> (n-1/2)
    if(node->isLeaf){
        minNumberofKeys = getSplitPoint(tree->nodeSize);
    }
    else{
        //n/2 - 1
        minNumberofKeys = getSplitPoint(tree->nodeSize + 1) - 1;
    }
    
    //simple case - node has still enough keys
    if (node->numOfKeys >= minNumberofKeys){
        return;
    }
    
    //node falls below minimum.Either merge or redistribute
    // Find the appropriate neighbor node with which to merge.
    //Also finds the key (kPrime) in the parent between the pointer to node n and the pointer to the neighbor.
    
    neighbourIndex = getNeighbourIndex(node);
    
    if(neighbourIndex ==-1){
        kIndex = 0;
    }
    else{
        kIndex = neighbourIndex;
    }
    
    innerKeyPrime = node->parent->keys[kIndex];
    
    
    if(neighbourIndex == -1){
        neighbor = node->parent->pointers[1];
    }else{
        neighbor = node->parent->pointers[neighbourIndex];
    }
    
    if(node->isLeaf){
        capacity = tree->nodeSize + 1;
    }
    else{
        capacity = tree->nodeSize;
    }
    
    
    //Merge - both nodes together have enough space
    if ((neighbor->numOfKeys + node->numOfKeys ) < capacity){
        mergeNodes(tree, node, neighbor, neighbourIndex, innerKeyPrime);
    }
    
    //Redistribution
    else{
        redestributeNodes(tree, node, neighbor, neighbourIndex, kIndex, innerKeyPrime);
    }
}

void redestributeNodes(BPlusTree * tree, Node * node, Node * neighbor, int neighbourIndex, int kIndex, double kPrime){
    
    int i;
    Node * tmp;
    
    //node has a neighbor to the left. Pull the neighbor's last key-pointer pair over from the neighbor's right end to n's left end.
    if (neighbourIndex != -1) {
        
        if(!node->isLeaf){
            node->pointers[node->numOfKeys + 1] = node->pointers[node->numOfKeys];
        }
        
        for (i = node->numOfKeys; i > 0; i--) {
            node->keys[i] = node->keys[i - 1];
            node->pointers[i] = node->pointers[i - 1];
            
        }
        
        if (!node->isLeaf) {
            node->pointers[0] = neighbor->pointers[neighbor->numOfKeys];
            tmp = (Node *)node->pointers[0];
            tmp->parent = node;
            neighbor->pointers[neighbor->numOfKeys] = NULL;
            node->keys[0] = kPrime;
            
            node->parent->keys[kIndex] = neighbor->keys[neighbor->numOfKeys-1];
            
        }
        else {
            
            node->keys[0] = neighbor->keys[neighbor->numOfKeys - 1];
            node->pointers[0] = neighbor->pointers[neighbor->numOfKeys - 1];
            
            node->parent->keys[kIndex] = node->keys[0];
        }
    }
    
    //node is the leftmost child. Take a key-pointer pair from the neighbor to the right.
    // Move the neighbor's leftmost key-pointer pair
    // to n's rightmost position.
    else {
        if (node->isLeaf) {
            node->keys[node->numOfKeys] = neighbor->keys[0];
            node->pointers[node->numOfKeys] = neighbor->pointers[0];
            
            node->parent->keys[kIndex] = neighbor->keys[1];
        }
        else {
            node->keys[node->numOfKeys] = kPrime;
            node->pointers[node->numOfKeys + 1] = neighbor->pointers[0];
            tmp = (Node *)node->pointers[node->numOfKeys + 1];
            tmp->parent = node;
            node->parent->keys[kIndex] = neighbor->keys[0];
        }
        for (i = 0; i < neighbor->numOfKeys - 1; i++) {
            neighbor->keys[i] = neighbor->keys[i + 1];
            neighbor->pointers[i] = neighbor->pointers[i + 1];
        }
        if (!node->isLeaf){
            neighbor->pointers[i] = neighbor->pointers[i + 1];
        }
    }
    
    //n now has one more key and one more pointer the neighbor has one fewer
    node->numOfKeys++;
    neighbor->numOfKeys--;
    
}

void mergeNodes(BPlusTree *tree, Node *node, Node *neighbor, int neighborIndex, double kPrime){
    
    int i, j, neighborInsertionIndex;
    Node * tmp;
    
    //Swap neighbor with node if node is on the extreme left and neighbor is to its right.
    if (neighborIndex == -1) {
        tmp = node;
        node = neighbor;
        neighbor = tmp;
    }
    
    //Starting point in the neighbor for copying keys and pointers from the other node.
    //Recall that n and neighbor have swapped places
    //in the special case of the node being a leftmost child
    neighborInsertionIndex = neighbor->numOfKeys;
    
    //nonleaf node.
    //Append k_prime and the following pointer.
    //append all pointers and keys from the neighbor.
    
    if(!node->isLeaf)
    {
        
        //key
        neighbor->keys[neighborInsertionIndex] = kPrime;
        neighbor->numOfKeys++;
        
        int decreasingIndex = 0;
        int numOfKeysBefore = node->numOfKeys;
        
        for (i = neighborInsertionIndex + 1, j = 0; j < node->numOfKeys; i++, j++) {
            neighbor->keys[i] = node->keys[j];
            neighbor->pointers[i] = node->pointers[j];
            neighbor->numOfKeys++;
            
            //decreases numOfKeys
            decreasingIndex++;
        }
        node->numOfKeys = numOfKeysBefore - decreasingIndex;
        neighbor->pointers[i] = node->pointers[j];
        
        //All children must now point up to the same parent.
        for (i = 0; i < neighbor->numOfKeys + 1; i++) {
            tmp = (Node *)neighbor->pointers[i];
            tmp->parent = neighbor;
        }
    }
    // a leaf, append the keys and pointers of the node to the neighbor.
    //Set the neighbor's last pointer to point to what had been the node's right neighbor.
    else
    {
        
        for (i = neighborInsertionIndex, j = 0; j < node->numOfKeys; i++, j++) {
            neighbor->keys[i] = node->keys[j];
            //ADD
            neighbor->pointers[i] = node->pointers[j];
            neighbor->numOfKeys++;
        }
        
        //relink leafs
        if(node->next != NULL){
            (node->next)->prev = neighbor;
        }
        neighbor->next = node->next;
    }
    
    deleteEntry(tree, node->parent, kPrime, node);
    
    free(node->keys);
    free(node->pointers);
    free(node);
    
}

int getNeighbourIndex(Node * node ){
    
    int i;
    // Return the index of the key to the left of the pointer in the parent pointing to the node
    // If node is not found return -1
    if(node->parent != NULL){
        
        for (i = 0; i <= node->parent->numOfKeys; i++){
            
            if (node->parent->pointers[i] == node){
                return i-1;
            }
        }
    }
    
    //node is leftmost child
    return -1;
}

void adjustTheRoot(BPlusTree *tree){
    
    Node * newRoot;
    
    //enough keys in root
    if (0 < tree->root->numOfKeys){
        return;
    }
    
    // If it has a child, promote the first (only) child as the new root.
    if (!tree->root->isLeaf) {
        newRoot = tree->root->pointers[0];
        newRoot->parent = NULL;
    }
    else{
        newRoot = NULL;
    }
    
    free(tree->root->keys);
    free(tree->root->pointers);
    free(tree->root);
    
    tree->root = newRoot;
}

void delete(BPlusTree *tree, timeStampT time, double value){
    
    Node * leaf;
    //default value for Multiple Times in Doubly linked List
    bool hasMultipleTimes = false;
    
    leaf = findLeaf(tree, value);
    
    int positionOfKey = -1;
    positionOfKey = findLeafKeyIndexAndSetboolIfMultipleListValues(leaf, value, &hasMultipleTimes);
    
    if(positionOfKey >= 0){
        
        //key has duplicates --> delete first value ind doubly linked list
        if(hasMultipleTimes){
            deleteFirstListValue(leaf, positionOfKey);
        }
        else{
            deleteEntry(tree, leaf, leaf->keys[positionOfKey], NULL);
            
        }
        
    }
#ifdef DEBUG
    else{
        printf("ERROR - RECORD THAT SHOULD GET DELETED IS NOT IN TREE");
    }
#endif
}

Node * removeEntryFromTheNode(BPlusTree *tree, Node * node, double toDelete, Node * pointerNode){
    
    // Remove the key and shift other keys
    int i = 0;
    
    while (node->keys[i] != toDelete){
        i++;
    }
    
    if(node->isLeaf){
        free(node->pointers[i]);
    }
    
    for (++i; i < node->numOfKeys; i++){
        node->keys[i - 1] = node->keys[i];
        //timestamps to
        if(node->isLeaf){
            node->pointers[i-1] = node->pointers[i];
        }
    }
    
    
    int y, numOfPointers;
    
    // Remove the pointer and shift other pointers accordingly.
    // First determine number of pointers.
    if(!node->isLeaf){
        
        numOfPointers = node->numOfKeys+1;
        
        y = 0;
        while (node->pointers[y] != pointerNode) {
            y++;
        }
        for (++y; y < numOfPointers; y++){
            node->pointers[y - 1] = node->pointers[y];
        }
        
    }
    //one key is gone
    node->numOfKeys--;
    
    // Set the other pointers to NULL for tidiness.
    // A leaf uses the last pointer to point to the next leaf.
    if (!node->isLeaf){
        for (i = node->numOfKeys + 1; i < tree->nodeSize + 1; i++)
            node->pointers[i] = NULL;
    }
    //leafnode
    else{
        //ADD
        for (i = node->numOfKeys + 1; i < tree->nodeSize; i++){
            node->pointers[i] = NULL;
        }
    }
    
    return node;
}

int findLeafKeyIndexAndSetboolIfMultipleListValues(Node * node, double key, bool *hasMultipleTimes){
    
    int i;
    for(i = 0; i< node->numOfKeys; i++){
        double currentKey = node->keys[i];
        
        if(key == currentKey){
            //first value in leaf --> is on the same pointer value
            ListValue * firstListValue = node->pointers[i];
            
            if(firstListValue->next != firstListValue){
                *hasMultipleTimes = true;
            }
            
            // position of pointer to timestamps
            return i;
        }
    }
    
    return -1;
    
}

void deleteFirstListValue(Node * leaf, int index){
    
    //first value in leaf
    ListValue * firstListValue = leaf->pointers[index];
    
    ListValue * next = firstListValue->next;
    ListValue * prev = firstListValue->prev;
    
    //next is the second oldest key
    leaf->pointers[index] = next;
    prev->next = next;
    next->prev = prev;
    
    //always the first list value must be the oldest list value and
    // therefore the one that is deleted
    
    free(firstListValue);
    
    
}

/************************************* COMMONLY USED METHODS **************************************/

Node * findLeaf(BPlusTree *tree, double newKey){
    int i = 0;
    
    Node * curNode = tree->root;
    
    
    if (curNode == NULL) {
        return curNode;
    }
    
    while (!curNode->isLeaf) {
        
        i = getInsertPoint(tree, curNode, newKey);
        //new lookup node
        curNode = (Node *)curNode->pointers[i];
    }
    
    //leaf found
    return curNode;
}

// Finds the place to split a node that is too big into two.
int getSplitPoint(int length) {
    if (length % 2 == 0){
        return length/2;
    }
    else{
        return length/2 + 1;
    }
}

/************************************* INSERTION **************************************/

void addRecordToTree(BPlusTree *tree, timeStampT time, double value){
    
    Node *leaf;
    
    //the tree does not exist yet --> create tree
    if (tree->root == NULL){
        newTree(tree, time, value);
        return;
    }
    
    //find the right leaf -- if duplicate: insertinto leaf
    leaf = findLeaf(tree, value);
    
    
    //if duplicate -> insert to leaf as Doubly linked list value
    if(isDuplicateKey(leaf, time, value)){
        addDuplicateToDoublyLinkedList(leaf, time, value);
        
    }
    else if(leaf->numOfKeys < tree->nodeSize) {
        insertRecordIntoLeaf(tree, leaf, time, value);
    }
    else{
        //leaf must be split
        splitAndInsertIntoLeaves(tree, leaf, time, value);
    }
    
    
}

void addDuplicateToDoublyLinkedList(Node *node, timeStampT newTime, double duplicateKey){
    
    int i;
    
    for(i = 0; i<node->numOfKeys; i++){
        double currentKey = node->keys[i];
        
        if(duplicateKey == currentKey){
            break;
        }
    }
    
    ListValue * newListValue = listValue_new(newTime);
    
    //first value in leaf
    ListValue * firstListValue = node->pointers[i];
    
    ListValue * lastListValue = firstListValue->prev;
    
    
    //update doubly linked pointers
    firstListValue->prev = newListValue;
    lastListValue->next = newListValue;
    
    newListValue->prev = lastListValue;
    newListValue->next = firstListValue;
    
}

bool isDuplicateKey(Node * curNode, timeStampT newTime, double newKey){
    
    /**duplicate**/
    // key is equal -> go to this pointer and directly insert to list
    for(int i = 0; i < curNode->numOfKeys; i++){
        double currentKey = curNode->keys[i];
        
        if(newKey == currentKey){
            return true;
        }
    }
    return false;
    
}

void splitAndInsertIntoLeaves(BPlusTree *tree, Node *oldNode, timeStampT time, double firstValue){
    
    Node * newNode;
    int insertPoint, split, i, j;
    double *tempKeys;
    void **tempPointers;
    
    //new Leaf
    newNode = Leaf_new(tree->nodeSize);
    
    tempKeys = malloc((tree->nodeSize + 1)* sizeof(double));
    //ADD
    tempPointers = malloc((tree->nodeSize +1)* sizeof(void *));
    
    insertPoint = 0;
    int nrOfTempKeys = 0;
    
    insertPoint = getInsertPoint(tree, oldNode, firstValue);
    
    //fills the keys and pointers
    for (i = 0, j = 0; i < oldNode->numOfKeys; i++, j++) {
        
        //if value is entered in the first position: pointers needs to be moved 1 position
        if (j == insertPoint){
            j++;
        }
        
        //the value where the new record is added is not been filled yet / a gap is inserted
        tempKeys[j] = oldNode->keys[i];
        //ADD
        tempPointers[j] = oldNode->pointers[i];
        nrOfTempKeys++;
        
        
    }
    
    //enter the record to the right position
    tempKeys[insertPoint] = firstValue;
    
    //ADD
    ListValue * newTime = listValue_new(time);
    tempPointers[insertPoint] = newTime;
    nrOfTempKeys++;
    
    newNode->numOfKeys = 0;
    oldNode->numOfKeys = 0;
    
    //leaves: treenodeSize - internal node: order
    split = getSplitPoint(tree->nodeSize);
    
    //fill first leaf
    for (i = 0; i < split; i++) {
        //goes left (smaller values)
        oldNode->keys[i] = tempKeys[i];
        //ADD
        oldNode->pointers[i] = tempPointers[i];
        oldNode->numOfKeys++;
    }
    
    
    //fill second leaf
    for (j = 0, i = split; i < nrOfTempKeys; i++, j++) {
        newNode->keys[j] = tempKeys[i];
        //ADD
        newNode->pointers[j] = tempPointers[i];
        
        newNode->numOfKeys++;
    }
    
    if(oldNode->next != NULL){
        newNode->next = oldNode->next;
        (oldNode->next)->prev = newNode;
    }
    
    
    oldNode->next = newNode;
    newNode->prev = oldNode;
    newNode->parent = oldNode->parent;
    
    //the record to insert in upper node
    double keyForParent;
    keyForParent = newNode->keys[0];
    
    //free allocated memory of pointers
    free(tempKeys);
    free(tempPointers);
    
    insertIntoParent(tree, oldNode, keyForParent, newNode);
    
}

void insertIntoParent(BPlusTree *tree, Node *oldChild, double newKey, Node *newChild){
    
    
    int pointerPositionToLeftNode;
    Node * parent;
    
    parent = oldChild->parent;
    
    //new root
    if(parent == NULL){
        insertIntoANewRoot(tree, oldChild, newKey, newChild);
        return;
    }
    
    //Find the parents pointer from the old node
    pointerPositionToLeftNode = getLeftPointerPosition(parent, oldChild);
    
    //the new key fits into the node
    if (parent->numOfKeys < tree->nodeSize){
        insertIntoTheNode(parent, pointerPositionToLeftNode, newKey, newChild);
    }
    
    else{
        //split a node in order to preserve the B+ tree properties*/
        splitAndInsertIntoInnerNode(tree, parent, pointerPositionToLeftNode, newKey, newChild);
    }
    
    
}

void insertIntoTheNode(Node * node, int index, double value, Node * newChild){
    
    double newInnerKey = value;
    
    for(int i = node->numOfKeys; i > index; i--) {
        
        //moves pointer one right till insertion point
        node->pointers[i + 1] = node->pointers[i];
        
        //because index is one too much (numOfKeys > index of last elemement)-> space for the new key
        node->keys[i] = node->keys[i - 1];
        
    }
    
    node->pointers[index +1] = newChild;
    node->keys[index] = newInnerKey;
    node->numOfKeys++;
    
    
}

//Inserts a new key and pointer to a node into a node, then the node's size exceeds the max nodeSize -> split
void splitAndInsertIntoInnerNode(BPlusTree *tree, Node * oldInnerNode, int index, double key, Node * childNode) {
    
    Node * newInnerNode;
    int split, i, j;
    double *tempKeys;
    void **tempPointers;
    int  nrOfTempKeys = 0;
    
    newInnerNode = Node_new(tree->nodeSize);
    
    //tempKeys and pointer are temporarly filled with up to nodesize + 1 keys
    tempKeys = malloc((tree->nodeSize +1) * sizeof(double));
    tempPointers = malloc((tree->nodeSize+2) * sizeof(void *));
    
    //fills the keys
    for (i = 0, j = 0; i < oldInnerNode->numOfKeys; i++, j++) {
        if(j == index){
            j++;
        }
        tempKeys[j] = oldInnerNode->keys[i];
        nrOfTempKeys++;
    }
    
    //fills the pointers
    for (int i = 0, j = 0; i < oldInnerNode->numOfKeys + 1; i++, j++) {
        if(j == index + 1){
            j++;
        }
        tempPointers[j] = oldInnerNode->pointers[i];
    }
    
    double newInnerKey = key;
    
    tempKeys[index] = newInnerKey;
    tempPointers[index + 1] = childNode;
    nrOfTempKeys++;
    
    newInnerNode->numOfKeys = 0;
    oldInnerNode->numOfKeys = 0;
    
    //innerNode splitpoint
    split = getSplitPoint(tree->nodeSize + 1);
    
    int x = 0;
    for (;x < split; x++) {
        oldInnerNode->keys[x] = tempKeys[x];
        oldInnerNode->pointers[x] = tempPointers[x];
        oldInnerNode->numOfKeys++;
    }
    oldInnerNode->pointers[x] = tempPointers[x];
    double leftMostKey = tempKeys[x];
    
    newInnerNode->parent = oldInnerNode->parent;
    
    newInnerNode->numOfKeys = nrOfTempKeys-oldInnerNode->numOfKeys-1;
    
    //one node is given to upper node and has not to be inserted in this one
    for (++x, j = 0; j < newInnerNode->numOfKeys; j++, x++) {
        //first key is not inserted to this node - it is inserted to upper node
        newInnerNode->pointers[j] = tempPointers[x];
        newInnerNode->keys[j] = tempKeys[x];
    }
    newInnerNode->pointers[j] = tempPointers[x];
    
    
    //set parent to new node
    for (int i = 0; i < newInnerNode->numOfKeys + 1; i++) {
        Node * childOfNewNode = newInnerNode->pointers[i];
        childOfNewNode->parent = newInnerNode;
    }
    
    //free allocated memory
    free(tempKeys);
    free(tempPointers);
    
    //old node to the left and new Node to the right
    insertIntoParent(tree, oldInnerNode, leftMostKey, newInnerNode);
    
    
}

//used in insertIntoParent to find the index of the pointer to the node to the left of the key to be inserted
int getLeftPointerPosition(Node * parent, Node * oldChild){
    
    int leftIndex = 0;
    //gets the index where the parent shows to the old, known child - after the new one will have the pointer leftIndex + 1
    
    while (parent->pointers[leftIndex] != oldChild){
        leftIndex++;
    }
    return leftIndex;
}

void insertIntoANewRoot(BPlusTree *tree, Node * left, double key, Node * right) {
    
    Node * root = Node_new(tree->nodeSize);
    tree->root = root;
    root->keys[0] = key;
    left->parent = root;
    right->parent = root;
    root->pointers[0] = left;
    root->pointers[1] = right;
    root->numOfKeys = 1;
    root->parent = NULL;
}

void insertRecordIntoLeaf(BPlusTree *tree, Node *node, timeStampT time, double newKey){
    
    int i;
    int insertPoint = 0;
    
    //duplicate keys have already been inserted therefore <
    
    insertPoint = getInsertPoint(tree, node, newKey);
    
    //update Keys
    for (i = node->numOfKeys; i > insertPoint; i--) {
        //keys are moved one right to make space for new record
        node->keys[i] = node->keys[i - 1];
        //ADD
        node->pointers[i] = node->pointers[i - 1];
    }
    
    ListValue *firstLValue = listValue_new(time);
    
    //no pointers necassary to update because node is directly inserted
    node->keys[insertPoint] = newKey;
    node->pointers[insertPoint] = firstLValue;
    
    
    node->numOfKeys++;
    
    
}

int getInsertPoint(BPlusTree *tree, Node *oldNode, double keyTimePair){
    int insertPoint = 0;
    
    while (oldNode->numOfKeys > insertPoint && oldNode->keys[insertPoint] <= keyTimePair){
        insertPoint++;
    }
    
    return insertPoint;
}

//create a new Tree
void newTree(BPlusTree *tree, timeStampT time, double value){
    tree->root = Leaf_new(tree->nodeSize);
    ListValue *newTimeListValue = listValue_new(time);
    tree->root->keys[0] = value;
    tree->root->pointers[0] = newTimeListValue;
    tree->root->parent = NULL;
    tree->root->numOfKeys = 1;
}

/******************************PRINT ARRAY*******************************************************************/

void printArray(CircularArray * array, int arraySize){
    
    printf("Elements in Serie:\n");
    
    for (int i = 0; i < arraySize; i++) {
        printf("%.2fl ", array->data[i].value);
    }
    printf("\n");
}

/************************************* CIRCULAR ARRAY ********************************************/

void serie_update(BPlusTree *tree, CircularArray *array, timeStampT newTime, double newValue){
    
    int newUpdatePosition = 0;
    
    if(array->count < array->size){
        if(array->count != 0){
            newUpdatePosition = (array->lastUpdatePosition + 1) %array->size;
        }
        array->count++;
    }
    else{
        newUpdatePosition = (array->lastUpdatePosition + 1) %array->size;
        delete(tree, array->data[newUpdatePosition].time, array->data[newUpdatePosition].value);
        
    }
    
    array->data[newUpdatePosition].time = newTime;
    array->data[newUpdatePosition].value = newValue;
    array->lastUpdatePosition = newUpdatePosition;
    
    addRecordToTree(tree, newTime, newValue);
    
}

bool lookup(CircularArray *array, timeStampT t, double *value){
    
    if(array->count == 0){
        return false;
    }
    
    //vom letzten Zeitpunkt ausgehen!!!
    int step = (int)(t - array->data[array->lastUpdatePosition].time)/TIMESTAMP_DIFF;
    
    if(abs(step) < array->count){
        //chaining : modulo for negative numbers
        int pos = (((array->lastUpdatePosition+step)%array->size)+array->size)%array->size;
        
        if(array->data[pos].time == t){
            *value = array->data[pos].value;
            return true;
        }
    }
    
    return false;
}

