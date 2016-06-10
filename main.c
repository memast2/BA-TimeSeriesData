//
//  main.c
//  BPlusTree
//  Created by Melina Mast on 20.03.16. based on implementation of http://www.amittai.com/prose/bpt.c and Database Systems:The Complete Book

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>


/******************************TIMESTAMP DEF********************************************/



#define timestampDiff 300

//t = type
typedef unsigned long timestamp_t;



/******************************STRUCTS - CIRCULAR ARRAY********************************************/


/* serie is element of circular array */
typedef struct serie{
    timestamp_t time;
    double value;
}serie;



typedef struct circularArray{
    serie *data;
    int size;
    int lastUpdatePosition;
}CircularArray;

CircularArray *CircularArray_new(int size) {
    CircularArray *array = NULL;
    array = malloc(sizeof(CircularArray));
    array->size = size;
    array->lastUpdatePosition = -1;
    array->data = malloc(array->size * sizeof(serie));
    return array;
}

void CircularArray_destroy(CircularArray *array) {
    free(array->data);
    free(array);
}



/******************************STRUCTS - DOUBLY LINKED LIST AND LEAF KEYS********************************************/


typedef struct listValue{
    timestamp_t timestamp;
    struct listValue *prev, *next;
}listValue;


listValue *listValue_new(timestamp_t time) {
    listValue *newListValue = NULL;
    newListValue = malloc(sizeof(listValue));
    newListValue->timestamp = time;
    newListValue->next = NULL;
    newListValue->prev = NULL;
    return newListValue;
}

typedef struct leafKey{
    double leafKey;
    struct listValue *firstListValue;
}leafKey;

leafKey *leafKey_new(double key, timestamp_t time) {
    
    leafKey *newLeafKey = NULL;
    newLeafKey = malloc(sizeof(leafKey));
    newLeafKey->leafKey = key;
    listValue newListValue = *listValue_new(time);
    newLeafKey->firstListValue = &newListValue;
   
    return newLeafKey;
}

void listValue_destroy(listValue *listValue) {
    free(listValue);
}

void leafKey_destroy(leafKey *leafKey) {
    listValue_destroy(leafKey->firstListValue);
    free(leafKey);
}


/******************************STRUCTS - BPLUSTREE*******************************************************************/


typedef struct Record{
    double recordKey;
    timestamp_t time;
}Record;

void Record_destroy(Record *record) {
    free(record);
}

Record *record_new(timestamp_t time, double value) {
    Record *newRecord = NULL;
    newRecord = malloc(sizeof(Record));
    newRecord->recordKey = value;
    newRecord->time = time;
    return newRecord;
}


typedef struct Node{
    struct Node *parent;
    void **pointers;
    int numOfKeys;
    //can contain records or innerNodes
    void ** keys;
    bool is_Leaf;
    //for leafs
    struct Node *prev, *next;
}Node;

typedef struct innerKey{
    double key;
}innerKey;

innerKey *innerKey_new(double key){
    innerKey *newInnerKey = NULL;
    newInnerKey = malloc(sizeof(innerKey));
    return newInnerKey;
}

void innerKey_destroy(innerKey *key){
    free(key);
}

Node * Node_new(int nodeSize){
    Node *node = malloc(sizeof(Node));
    node->keys = malloc(nodeSize * sizeof(void));
    //always n+1 pointers
    node->pointers = malloc((nodeSize + 1) * sizeof(void));
    node->next = NULL;
    node->prev = NULL;
    return node;
}

Node *Leaf_new(int nodeSize){
    Node *node = Node_new(nodeSize);
    node->is_Leaf = true;
    return node;
}

void Node_destroy(Node *node) {
    free(node->pointers);
    free(node->parent);
    free(node->keys);
    free(node);
}

typedef struct BPlusTree{
    struct Node *root;
    int nodeSize;
}BPlusTree;

BPlusTree *BPlusTree_new(int nodeSize) {
    BPlusTree *bPlus = NULL;
    bPlus = malloc(sizeof(BPlusTree));
    bPlus->nodeSize = nodeSize;
    bPlus->root = malloc(sizeof(Node));
    bPlus->root = NULL;
    return bPlus;
}

void BPlusTree_destroy(BPlusTree *bPlus) {
    Node_destroy(bPlus->root);
    //TODO GANZEN BAUM DURCHGEHEN
    free(bPlus);
}


bool lookup(CircularArray *array, timestamp_t t, double *value);
void shift(BPlusTree *tree, CircularArray *array, timestamp_t time, double value);
timestamp_t neighbour(double value, timestamp_t t);

void printArray(CircularArray * array, int arraySize);
void serie_update(BPlusTree *tree, CircularArray *array, timestamp_t t, double value);
void initialize_data(CircularArray *array);
void initialize_tree(BPlusTree *tree);


void deleteRecordFromTree(BPlusTree *tree, timestamp_t t, double value);


void addRecordToTree(BPlusTree *tree, timestamp_t t, double value);
void newTree(BPlusTree *tree, timestamp_t time, double value);
Node * findLeaf(BPlusTree *tree, double newKey, timestamp_t newTime);
void insertRecordIntoLeaf(BPlusTree *tree, Node *leaf, leafKey *firstValue);
int getSplitPoint(int length);
void insertIntoParent(BPlusTree *tree, Node *oldChild, double newKey, Node *newChild);
void insertIntoANewRoot(BPlusTree *tree, Node * left, double record, Node * right);
void splitAndInsertIntoInnerNode(BPlusTree *tree, Node * parent, int leftIndex, double key, Node * rightNode);
int getLeftPointerPosition(Node * parent, Node * left);
void splitAndInsertIntoLeaves(BPlusTree *tree, Node *oldNode, leafKey *firstValue);
void insertIntoNode(BPlusTree *tree, Node * parent, int leftIndex, double record, Node * newChild);

Record * findRecordandRecordLeaf(BPlusTree *tree, double value);
Node * deleteEntry(BPlusTree *tree, double value, Node *leaf);
Node * removeEntryFromTheNode(BPlusTree *tree, double value, Node * leaf);
Node * adjustTheRoot(BPlusTree *tree);
Node * mergeNodes(BPlusTree *tree, Node *node, Node *neighbor, int neighborIndex, double key);
Node * redestributeNodes(BPlusTree *tree, Node *node, Node *neighbor, int neighborIndex, int kIndex, double key);


int getPointerIndex(Node * node);
void printTree(BPlusTree *tree);
void addDuplicateToDoublyLinkedList(timestamp_t newTime, Node *node, int insertIndex);
int getInsertPoint(BPlusTree *tree, Node *oldNode, leafKey *keyTimePair);


int main(int argc, const char * argv[]) {
    
    //variable - can be commandLine Input
    int arraySize =3;
    int treeNodeSize = 3;
    timestamp_t ti = 300;
    
    CircularArray *array = CircularArray_new(arraySize);
    initialize_data(array);

    //create BplusTree;
    BPlusTree *tree = BPlusTree_new(treeNodeSize);
    
    shift(tree, array, ti, 300);
    addRecordToTree(tree, 600, 600);
    

    printTree(tree);


    
    
    BPlusTree_destroy(tree);
    CircularArray_destroy(array);

    return 0;
}

void shift(BPlusTree *tree, CircularArray *array, timestamp_t time, double value){
    
    //just new values are added to the tree and to the circular array
     addRecordToTree(tree, time, value);
     serie_update(tree, array, time, value);
    
}

void printArray(CircularArray * array, int arraySize){

    printf("Elements in Serie:\n");
    
    for (int i = 0; i < arraySize; i++) {
        printf("%.2fl ", array->data[i].value);
    }
    printf("\n");
}

void printTree(BPlusTree *tree){
    printf("\nPrint Tree: ");
    printf("\nroot: ");
    
    Node *root = tree->root;
    
    for(int i=0; i < root->numOfKeys; i++){
        printf(" %fl ",((innerKey *)root->keys[i])->key);
    }
    printf("\n");
    
    if(NULL != tree->root->pointers){

        printf("\n first level: ");
    for(int i = 0; i< tree->root->numOfKeys+1;i++){
        if(NULL != tree->root->pointers[0]){
            Node * node = tree->root->pointers[0];
            printf("\n Node in level: %d ", i);
            if(NULL != node->next){
                node = node->next;
   //         for(int j = 0; j< node->numOfKeys; j++){
    //            printf("%fl ",((Record *)node->keys[j])->recordKey);
      //          }
            }
      

        }
        
    }

    }
}
/*

Node * deleteEntry(BPlusTree *tree, double value, Node *node){
    
    int minNumberofKeys;
    Node * neighbor;
    int neighborIndex, pointerIndex;
    int kIndex;
    double key;
    int capacity;
    
    // Remove key and pointer from node.
    node = removeEntryFromTheNode(tree, value, node);
    
    if (node == tree->root)
        return adjustTheRoot(tree);
    
    
    // deletion from a node below the root
    //TODO find out split point
    minNumberofKeys = getSplitPoint(tree->nodeSize + 1);

    if (node->numOfKeys >= minNumberofKeys){
        return tree->root;
    }
    
    // Find the appropriate neighbor node with which to merge or redestribute.
    pointerIndex = getPointerIndex(node);
    neighborIndex = pointerIndex -1;
    
    kIndex = pointerIndex - 1 == -1 ? 0 : neighborIndex;
    key = ((innerKey *)node->parent->keys[kIndex])->key;
    neighbor = neighborIndex == -1 ? node->parent->pointers[1] : node->parent->pointers[neighborIndex];
    
    capacity = tree->nodeSize;
    
    
    //Merge
    if (neighbor->numOfKeys + node->numOfKeys < capacity)
        return mergeNodes(tree, node, neighbor, neighborIndex, key);
    
    //Redistribution
    else
        return redestributeNodes(tree, node, neighbor, neighborIndex, kIndex, key);
}

Node * mergeNodes(BPlusTree *tree, Node *node, Node *neighbor, int neighborIndex, double key){
    return node;
}

Node * redestributeNodes(BPlusTree *tree, Node *node, Node *neighbor, int neighborIndex, int kIndex, double key){
    return node ;
}


Node * removeEntryFromTheNode(BPlusTree *tree, double value, Node * node){
    
    int i, numPointers;
    
    // Remove the key and shift other keys
    i = 0;
    while (((innerKey *)node->keys[i])->key != value){
        i++;
    }
    for (++i; i < node->numOfKeys; i++)
        node->keys[i - 1] = node->keys[i];
    
    // Remove the pointer and shift other pointers accordingly.
    // First determine number of pointers.
    numPointers = node->numOfKeys+1;
   
    //how to handle pointers
   // while (node->pointers[i] != pointer)
   //     i++;
    for (++i; i < numPointers; i++)
        node->pointers[i - 1] = node->pointers[i];
    
    node->numOfKeys--;
    
    // Set the other pointers to NULL
    if (!node->is_Leaf){
        for (i = node->numOfKeys + 1; i < tree->nodeSize+1; i++){
            node->pointers[i] = NULL;

        }
    }
  
    return node;
}

Node * adjustTheRoot(BPlusTree *tree){
    
    Node * newRoot;
    
    //Key and pointer have already been deleted
    if (0 < tree->root->numOfKeys){
        return tree->root;
    }
    
    // If it has a child, promote the first (only) child as the new root.
    if (!tree->root->is_Leaf) {
        newRoot = tree->root->pointers[0];
        newRoot->parent = NULL;
    }
    else
        newRoot = NULL;
    
    free(tree->root->keys);
    free(tree->root->pointers);
    free(tree->root);
    
    //evt nullpointer?
    tree->root = newRoot;
    
    return newRoot;
    
}

int getPointerIndex(Node * node ){
    
    int i;
    // Return the index of the key to the the pointer in the parent pointing to n.
    // If node is not found return -1
    for (i = 0; i <= node->parent->numOfKeys; i++){
        if (node->parent->pointers[i] == node){
            return i;
        }
    }
    return -1;
}

void deleteRecordFromTree(BPlusTree *tree, timestamp_t time, double value) {
    
    Node * keyLeaf;
    Record * keyRecord;
    printf("\nfirst ADDRESS OF LEAF: %d", (int)&keyLeaf);

    keyLeaf = findLeaf(tree, value, time);
    //TODO: check if keyLeaf cannot be addressed thru &
    keyRecord = findRecordandRecordLeaf(tree, value);
    
    if (keyLeaf != NULL && keyRecord != NULL) {
        tree->root = deleteEntry(tree, value, keyLeaf);
        free(keyRecord);
    }
}

    Record * findRecordandRecordLeaf(BPlusTree *tree, double value) {
   int i = 0;
Node * leaf = findLeaf(tree, value, time);
    
   if (leaf == NULL) return NULL;
    
    for (i = 0; i < leaf->numOfKeys; i++){
        
        if (((Record *)leaf->keys[i])->recordKey == value) break;
    }
    if (i == leaf->numOfKeys)
        return NULL;
    else
        return (Record *)leaf->pointers[i];


}   */


/************************************* INSERTION **************************************/

void addRecordToTree(BPlusTree *tree, timestamp_t time, double value){
    
    Node *leaf;

    //the tree does not exist yet --> create tree
    if (tree->root == NULL){
        newTree(tree, time, value);
        return;
    }
    
    //find the right leaf -- if duplicate: insertinto leaf
    leaf = findLeaf(tree, value, time);
    
    leafKey *leafKeyValue = leafKey_new(time, value);

    
    // leaf has room for key and pointer
    if (leaf->numOfKeys < tree->nodeSize) {
        
        insertRecordIntoLeaf(tree, leaf, leafKeyValue);
        return;
    }

    //leaf must be split
    splitAndInsertIntoLeaves(tree, leaf, leafKeyValue);
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

void splitAndInsertIntoLeaves(BPlusTree *tree, Node *oldNode,leafKey *firstValue){
    
    Node * newNode;
    int insertPoint, split, i, j;
    void **tempKeys;
    
    //new Leaf
    newNode = Leaf_new(tree->nodeSize);
    
    tempKeys = malloc(tree->nodeSize * sizeof(void));
    
    insertPoint = 0;
    int nrOfTempKeys = 0;

    insertPoint = getInsertPoint(tree, oldNode,firstValue);
    
    //fills the keys and pointers
    for (i = 0, j = 0; i < oldNode->numOfKeys; i++, j++) {
        
        //if value is entered in the first position: pointers needs to be moved 1 position
        if (j == insertPoint){
          j++;
        }
        
        nrOfTempKeys++;
        //the value where the new record is added is not been filled yet / a gap is inserted
        tempKeys[j] = oldNode->keys[i];

    }
    
    //enter the record to the right position
    tempKeys[insertPoint] = firstValue;
    nrOfTempKeys++;

    newNode->numOfKeys = 0;
    oldNode->numOfKeys = 0;
    
    
    split = getSplitPoint(tree->nodeSize);
    
    //fill first leaf
    for (i = 0; i < split; i++) {
        //goes left (smaller values)
        oldNode->keys[i] = tempKeys[i];
        oldNode->numOfKeys++;
    }
    
    printf("\nNrOfTempKeys: %d", nrOfTempKeys);
    printf("\nKeys in new node: ");
    
    
    //fill second leaf
    for (j = 0, i = split; j < nrOfTempKeys-split; i++, j++) {
        
        printf("%fl", ((leafKey *)newNode->keys[j])->leafKey);

        
        newNode->keys[j] = tempKeys[i];
        newNode->numOfKeys++;
    }
    
    if(oldNode->next != NULL){
        newNode->next = oldNode->next;
        oldNode->next->prev = newNode;
    }
    
    
    oldNode->next = newNode;
    newNode->prev = oldNode;
    newNode->parent = oldNode->parent;
    
    //the record to insert in upper node
    double keyForParent = ((leafKey *)newNode->keys[0])->leafKey;
    
    insertIntoParent(tree, oldNode, keyForParent, newNode);
    
    //free allocated memory of pointers
    free(tempKeys);

}

void insertIntoParent(BPlusTree *tree, Node *oldChild, double newKey, Node *newChild){
    
    int pointerPositionToLeftNode;
    Node *parent;
    
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
        
        insertIntoNode(tree, parent, pointerPositionToLeftNode, newKey, newChild);
        
        return;
    }

    //split a node in order to preserve the B+ tree properties*/
    splitAndInsertIntoInnerNode(tree, parent, pointerPositionToLeftNode, newKey, newChild);
    
}

//Inserts a new key and pointer to a node into a node, then the node's size exceeds the max nodeSize -> split
void splitAndInsertIntoInnerNode(BPlusTree *tree, Node * parent, int leftIndex, double key, Node * rightNode) {
    
    Node * newNode;
    int insertPoint, split, i, j;
    void **tempKeys;
    void **tempPointers;
    
    insertPoint = leftIndex + 1;
    newNode = Node_new(tree->nodeSize);
    
    tempKeys = malloc(tree->nodeSize * sizeof(innerKey));
    tempPointers = malloc((tree->nodeSize+1) * sizeof(void));
    
    //fills the keys and pointers
    for (i = 0, j = 0; i < parent->numOfKeys + 1; i++, j++) {
        if(j == insertPoint){
           j++;
        }
        if(i < parent->numOfKeys){
          tempKeys[j] = parent->keys[i];
        }
        tempPointers[j] = parent->pointers[i];
    }
    
    innerKey *newInnerKey = innerKey_new(key);
    
    tempKeys[leftIndex] = newInnerKey;
    tempPointers[insertPoint] = rightNode;
    
    newNode->numOfKeys = 0;
    split = getSplitPoint(tree->nodeSize);
    
    for (i = 0; i < split; i++) {
        parent->keys[i] = tempKeys[i];
        parent->pointers[i] = tempPointers[i];
        parent->numOfKeys++;
    }
    
    for (i = split, j = 0; i < tree->nodeSize; i++, j++) {
        newNode->keys[j] = tempKeys[i];
        newNode->pointers[i] = tempPointers[i];
        newNode->numOfKeys++;
    }
    
    //unused pointers
    for (i = parent->numOfKeys; i < tree->nodeSize; i++)
        parent->pointers[i] = NULL;
    for (i = newNode->numOfKeys; i < tree->nodeSize; i++)
        newNode->pointers[i] = NULL;
    
    newNode->parent = parent->parent;
    innerKey * leftMostKey = (innerKey *)newNode->keys[0];
   
    free(tempKeys);
    free(tempPointers);
    
    //old node to the left and new Node to the right
    insertIntoParent(tree, parent, leftMostKey->key, rightNode);
}

//used in insertIntoParent to find the index of the pointer to the node to the left of the key to be inserted
int getLeftPointerPosition(Node * parent, Node * oldChild){
    
    int leftIndex = 0;
    //gets the index where the parent shows to the old, known child - after the new one will have the pointer leftIndex + 1
    
    while (leftIndex <= parent->numOfKeys && (parent->pointers[leftIndex]) != oldChild){
        leftIndex++;
    }
    return leftIndex;
}

// Inserts a new key and pointer to a node
void insertIntoNode(BPlusTree *tree, Node * parent, int leftIndex, double record, Node * newChild) {
   
    innerKey *newInnerKey = innerKey_new(record);
    
    for (int i = parent->numOfKeys; i > leftIndex; i--) {
        
        //moves pointer one right till insertion point
        parent->pointers[i + 1] = parent->pointers[i];
        
        //because index is one too much (numOfKeys > index of last elemement)-> space for the new key
        parent->keys[i] = parent->keys[i - 1];
        
    }
    parent->pointers[leftIndex + 1] = newChild;
    parent->keys[leftIndex] = newInnerKey;
    parent->numOfKeys++;
    
}

void insertIntoANewRoot(BPlusTree *tree, Node * left, double key, Node * right) {
    
    Node * root = Node_new(tree->nodeSize);
    innerKey * newInnerKey = innerKey_new(key);
    root->keys[0] = newInnerKey;
    root->pointers[0] = left;
    root->pointers[1] = right;
    root->numOfKeys = 1;
    root->parent = NULL;
    left->parent = root;
    right->parent = root;
    tree->root = root;
}

void insertRecordIntoLeaf(BPlusTree *tree, Node *node, leafKey *newKey){
    
    int i;
    int insertPoint = 0;
    
    //duplicate keys have already been inserted therefore <
    
    insertPoint = getInsertPoint(tree, node, newKey);
   
    //update Keys
    for (i = node->numOfKeys; i > insertPoint; i--) {
        //keys are moved one right to make space for new record
        node->keys[i] = node->keys[i - 1];
    }
    
    //no pointers necassary to update because node is directly inserted
    node->keys[insertPoint] = newKey;
    node->numOfKeys++;
    
    //debugging
    printf("\nInsert Record into Leaf: elements in leaf: ");
    for(int y = 0; y< node->numOfKeys; y++){
        printf("%fl ", ((leafKey *)node->keys[y])->leafKey);
    }
    
}

Node * findLeaf(BPlusTree *tree, double newKey, timestamp_t newTime){
    int i = 0;
    Node * curNode = tree->root;
    
    
    if (curNode == NULL) {
        return curNode;
    }

    while (!curNode->is_Leaf) {

        i = 0;
        while (i < curNode->numOfKeys) {
            
            double currentKey = ((innerKey *)curNode->keys[i])->key;
                
             //key is bigger -> search again
            if (newKey > currentKey){
                  i++;
             }
            
            /**duplicate**/
             // key is equal -> go to this pointer and directly insert to list
            if(newKey == currentKey){
                addDuplicateToDoublyLinkedList(newTime, curNode, i);
            }
            
            //key is smaller -> go to this pointer
            else break;
        }
       
        //new lookup node
        curNode = (Node *)curNode->pointers[i];
    }
    
    //debugging
    printf("\nLeaf with num of keys: %d was [", curNode->numOfKeys);
    for (i = 0; i < curNode->numOfKeys; i++){
      printf("%fl ", ((Record *)curNode->keys[i])->recordKey);
    }
    
    //leaf found
    return curNode;
}

void addDuplicateToDoublyLinkedList(timestamp_t newTime, Node *node, int insertIndex){
    
    listValue newListValue = *listValue_new(newTime);
    
    leafKey currentLeafKey = *((leafKey *)node->keys[insertIndex]);
    listValue firstListValue = *currentLeafKey.firstListValue;
    
    listValue currentListValue = firstListValue;
    
    while (currentListValue.next != NULL) {
        currentListValue = *currentListValue.next;
    }
    
    //update doubly linked pointers
    currentListValue.next = &newListValue;
    
    newListValue.prev = &currentListValue;
    newListValue.next = &firstListValue;
    firstListValue.prev = &newListValue;
    
    return;
    
}

int getInsertPoint(BPlusTree *tree, Node *oldNode, leafKey *keyTimePair){
    int insertPoint = 0;
    
    while ((tree->nodeSize > oldNode->numOfKeys )&& oldNode->numOfKeys > insertPoint && ((leafKey *)oldNode->keys[insertPoint])->leafKey < keyTimePair->leafKey){
        insertPoint++;
    }
    
    return insertPoint;
}

//create a new Tree
void newTree(BPlusTree *tree, timestamp_t time, double value){
    tree->root = Leaf_new(tree->nodeSize);
    leafKey *record = leafKey_new(time, value);
    tree->root->keys[0] = record;
    tree->root->pointers = NULL;
    tree->root->parent = NULL;
    tree->root->numOfKeys = 1;
}


/************************************* CIRCULAR ARRAY ********************************************/

void serie_update(BPlusTree *tree, CircularArray *array, timestamp_t newTime, double newValue){
    
    int newUpdatePosition;
    int length = array->size;
    int lastUpdatePosition = array->lastUpdatePosition;
    timestamp_t minAllowedTime = 0;

    //because after a position cannot be negative
    if(array->lastUpdatePosition < 0){
        //Nothing has been added to the circularArray yet
        array->data[0].value = newValue;
        array->data[0].time = newTime;
        array->lastUpdatePosition = 0;
    }

    else{
        
        if(newTime>array->data[lastUpdatePosition].time){
            minAllowedTime = newTime - (array->size - 1) * timestampDiff;
        }
        else{
            minAllowedTime = array->data[lastUpdatePosition].time - (array->size - 1) * timestampDiff;
            if(minAllowedTime>=newTime){
                return;
            }
        }
        
        int positionStep = (int)(newTime - array->data[lastUpdatePosition].time)/timestampDiff;
        
        printf("last Update Pos was: %d\n", lastUpdatePosition);
        
        for (int i = 0; i<positionStep; i++) {
            
            int nextUpdatePosition = (lastUpdatePosition+i)%length;
            double value = array->data[nextUpdatePosition].value;
            timestamp_t curTime = array->data[nextUpdatePosition].time;
            
            //checks if the time is in the range of the circular Array after the new value is inserted to it
            //Range is: newTime - (array->size-1) * timeDifference
            
            // curTime is not always < minAllowedTime; minTime is the minimumAllowed Time which can stay in the serie, The other entries have to be deleted
            if(curTime < minAllowedTime && isfinite(value)){
                
                //deletes the record from the array and the b+tree
  //              deleteRecordFromTree(tree, curTime, value);
                
                array->data[nextUpdatePosition].value = INFINITY;
                array->data[nextUpdatePosition].time = INFINITY;
            }
 
        }
        
      //  timestamp_t timeDelete = array->data[newUpdatePosition].time;
      //  double valueDelete = array->data[newUpdatePosition].value;
        
     //   if(isfinite(valueDelete)){
    //          deleteRecordFromTree(tree, timeDelete, valueDelete);
   //     }
        
        newUpdatePosition = (lastUpdatePosition + positionStep)%length;
            
        array->data[newUpdatePosition].time = newTime;
        array->data[newUpdatePosition].value = newValue;
        
        array->lastUpdatePosition = newUpdatePosition;
        
              
        printf("Elements in Serie:\n");
        for (int i = 0; i < array->size; i++) {
            printf("%.2fl ", array->data[i].value);
        }
        printf("\n");
    
    }
}

bool lookup(CircularArray *array, timestamp_t t, double *value){
    
    //means array has min one record
    if(array->lastUpdatePosition>=0){
        
        //vom letzten Zeitpunkt ausgehen!!!
        int step = (int)(t - array->data[array->lastUpdatePosition].time)/timestampDiff;
        //chaining : modulo for negative numbers
        int pos = (((array->lastUpdatePosition+step)%array->size)+array->size)%array->size;
        
       if(array->data[pos].time == t){
           *value = array->data[pos].value;
           return true;
       }
    }

    return false;
}

void initialize_data(CircularArray *array){
    
    for (int i = 0;i< array->size; i++) {
        array->data[i].time = INFINITY;
        array->data[i].value = INFINITY;
    }
    
}
