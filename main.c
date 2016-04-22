//
//  main.c
//  BPlusTree
//  Created by Melina Mast on 20.03.16.

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

//maximum number of records:The minimum number of keys per record is 1/2 of the maximum number of keys. For example, if the order of a B+ tree is n, each node (except for the root) must have between n/2 and n keys.For a n-order B+ tree with a height of h:
//maximum number of keys is n^h
//minimum number of keys is 2(n/2)^{h-1}.



#define timestampDiff 300

//t = type
typedef unsigned long timestamp_t;

/* serie is element of circular array */
typedef struct serie{
    timestamp_t time;
    double value;
}serie;

typedef struct Record{
    double recordKey;
    timestamp_t time;
}Record;

Record *record_new(timestamp_t time, double value) {
    Record *newRecord = NULL;
    newRecord = malloc(sizeof(Record));
    newRecord->recordKey = value;
    newRecord->time = time;
    return newRecord;
}
void Record_destroy(Record *record) {
    free(record);
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
}innerNode;


Node *Node_new(int nodeSize){
    Node *node = NULL;
    node = malloc(sizeof(Node));
    node->keys = malloc(nodeSize * sizeof(void));
    //always n+1 pointers
    node->pointers = malloc((nodeSize + 1) * sizeof(void));
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
    //FRAGE: Hier müsste man den Baum durchgehen oder nicht?
    free(bPlus);
}


bool lookup(CircularArray *array, timestamp_t t, double *value);
void shift(BPlusTree *tree, CircularArray *array, timestamp_t time, double value);
timestamp_t neighbour(double value, timestamp_t t);
void serie_update(BPlusTree *tree, CircularArray *array, timestamp_t t, double value);
void initialize_data(CircularArray *array);
void initialize_tree(BPlusTree *tree);
void deleteRecordFromTree(BPlusTree *tree, timestamp_t t, double value);
void addRecordToTree(BPlusTree *tree, timestamp_t t, double value);
void newTree(BPlusTree *tree, Record *record);
Node * findLeaf(BPlusTree *tree, double key);
void insertRecordIntoLeaf(Node *node, Record *record);
int getSplitPoint(int length);
Node * insertIntoParent(BPlusTree *tree, Node *oldLeaf, Record *record, Node *newLeaf);
Node * insertIntoANewRoot(BPlusTree *tree, Node * left, Record *record, Node * right);
Node * insertIntoNode(BPlusTree *tree, Node * n, int leftIndex, Record *record, Node * right);
int getLeftPointerPosition(Node * parent, Node * left);
Node * splitAndInsertIntoInnerNode(BPlusTree *tree, Node * leftNode, int leftIndex, Record *record, Node * rightNode);
Node * splitAndInsertIntoLeaves(BPlusTree *tree, Node *oldNode, Record *record);
Record * findRecordandRecordLeaf(BPlusTree *tree, double value);
Node * deleteEntry(BPlusTree *tree, double value, Node *leaf);
Node * removeEntryFromTheNode(BPlusTree *tree, double value, Node * leaf);
Node * adjustTheRoot(BPlusTree *tree);
Node * mergeNodes(BPlusTree *tree, Node *node, Node *neighbor, int neighborIndex, double key);
Node * redestributeNodes(BPlusTree *tree, Node *node, Node *neighbor, int neighborIndex, int kIndex, double key);
int getPointerIndex(Node * node);



int main(int argc, const char * argv[]) {
    
    //variable - can be commandLine Input
    int arraySize =3;
    int treeNodeSize = 3;
    timestamp_t ti = 300;
    double lookupValue = 0;
    
    CircularArray *array = CircularArray_new(arraySize);
    initialize_data(array);

    //create BplusTree;
    BPlusTree *tree = BPlusTree_new(treeNodeSize);
    
    shift(tree, array, ti, 300);
    shift(tree, array, 600, 600);
    shift(tree, array, 900, 900);
    shift(tree, array, 1500, 1500);
    shift(tree, array, 2100, 2100);

    bool x1 = lookup(array, 300, &lookupValue);
    bool x2 = lookup(array, 600, &lookupValue);
    bool x3 = lookup(array, 1500, &lookupValue);
    bool x4 = lookup(array, 2100, &lookupValue);

    //1 means true
    printf("\nlookup:\n%d\n", x1);
    printf("%d\n", x2);
    printf("%d\n", x3);
    printf("%d\n", x4);
    printf("%fl\n", lookupValue);
    
    printf("Elements in Serie:\n");

    for (int i = 0; i < arraySize; i++) {
        printf("%.2fl ", array->data[i].value);
    }
    printf("\n");
    
    //BPlusTree_destroy(tree);
    CircularArray_destroy(array);

    return 0;
}

void shift(BPlusTree *tree, CircularArray *array, timestamp_t time, double value){
  // TODO
  //  addRecordToTree(tree, time, value);
    serie_update(tree, array, time, value);

}




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
    key = ((innerNode *)node->parent->keys[kIndex])->key;
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



//TODO CHECK POINTER
Node * removeEntryFromTheNode(BPlusTree *tree, double value, Node * node){
    
    int i, numPointers;
    
    // Remove the key and shift other keys
    i = 0;
    while (((innerNode *)node->keys[i])->key != value){
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
    printf("first ADDRESS OF LEAF: %d", (int)&keyLeaf);

    keyLeaf = findLeaf(tree, value);
    //TODO: check if keyLeaf cannot be addressed thru &
    keyRecord = findRecordandRecordLeaf(tree, value);
    
    if (keyLeaf != NULL && keyRecord != NULL) {
        tree->root = deleteEntry(tree, value, keyLeaf);
        free(keyRecord);
    }
}

Record * findRecordandRecordLeaf(BPlusTree *tree, double value) {
    int i = 0;
    Node * leaf = findLeaf(tree, value);
    
    if (leaf == NULL) return NULL;
    
    for (i = 0; i < leaf->numOfKeys; i++){
        
        if (((Record *)leaf->keys[i])->recordKey == value) break;
    }
    if (i == leaf->numOfKeys)
        return NULL;
    else
        return (Record *)leaf->pointers[i];

}


/************************* INSERTION **************************************/

void addRecordToTree(BPlusTree *tree, timestamp_t time, double value){
    
    Record *record = record_new(time, value);
    Node *leaf;

    //the tree does not exist yet
    if (tree->root == NULL){
       return newTree(tree, record);
    }
    
    //the tree already exists
    leaf = findLeaf(tree, record->recordKey);
    
    // leaf has room for key and pointer
    if (leaf->numOfKeys < tree->nodeSize) {
        return insertRecordIntoLeaf(leaf, record);
    }
    
    //leaf must be split
    splitAndInsertIntoLeaves(tree, leaf, record);
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

Node * splitAndInsertIntoLeaves(BPlusTree *tree, Node *oldNode, Record *record){
    
    Node * newNode;
    int insertPoint, split, i, j;
    void **tempKeys;
    newNode = Leaf_new(tree->nodeSize);
    
    tempKeys = malloc(tree->nodeSize * sizeof(void));
    insertPoint = 0;
    
    // <=because of possible duplicates?
    // calculation of the insertPoint for the new Record: as soon as key is higher insert point is right
    while (insertPoint < tree->nodeSize && ((Record *)oldNode->keys[insertPoint])->recordKey <= record->recordKey){
        insertPoint++;
    }
    
    //fills the keys and pointers
    for (i = 0, j = 0; i < oldNode->numOfKeys; i++, j++) {
        
        //if value is entered in the first position: pointers needs to be moved 1 position
        if (j == insertPoint) j++;
        //the value where the new record is added is not been filled yet
        tempKeys[j] = oldNode->keys[i];

    }
    
    //enter the record to the right position
    tempKeys[insertPoint] = record;

    newNode->numOfKeys = 0;
    split = getSplitPoint(tree->nodeSize);
    
    //fill first leaf
    for (i = 0; i < split; i++) {
        //goes left (smaller values)
        oldNode->keys[i] = tempKeys[i];
        oldNode->numOfKeys++;
    }
    
    //fill second leaf
    for (j = 0, i = split; i < tree->nodeSize; i++, j++) {
        newNode->keys[j] = tempKeys[i];
        newNode->numOfKeys++;
    }
    
    oldNode->next = newNode;
    newNode->next = oldNode->next;
    newNode->next->prev = newNode;
    newNode->prev = oldNode;
    
    newNode->parent = oldNode->parent;
    //the record is on the
    Record * newRecord = (Record *)newNode->keys[0];

    //free allocated memory of pointers
    free(tempKeys);
    
    return insertIntoParent(tree, oldNode, newRecord, newNode);
}

Node * insertIntoParent(BPlusTree *tree, Node *oldChild, Record *newRecord, Node *newChild){
    
    int pointerPositionToLeftNode;
    Node *parent;
    
    parent = oldChild->parent;
    
    //new root
    if (parent == NULL)
        return insertIntoANewRoot(tree, oldChild, newRecord, newChild);
    
    //Find the parents pointer from the old node
    pointerPositionToLeftNode = getLeftPointerPosition(parent, oldChild);
    
    
    //the new key fits into the node
    if (parent->numOfKeys < tree->nodeSize)
        return insertIntoNode(tree, parent, pointerPositionToLeftNode, newRecord, newChild);
    
    //split a node in order to preserve the B+ tree properties*/
    return splitAndInsertIntoInnerNode(tree, parent, pointerPositionToLeftNode, newRecord, newChild);

}

//Inserts a new key and pointer to a node into a node, then the node's size exceeds the max nodeSize -> split
Node * splitAndInsertIntoInnerNode(BPlusTree *tree, Node * parent, int leftIndex, Record *record, Node * rightNode) {
    
    Node * newNode;
    int insertPoint, split, i, j;
    void **tempKeys;
    void **tempPointers;
    
    insertPoint = leftIndex + 1;
    newNode = Node_new(tree->nodeSize);
    
    tempKeys = malloc(tree->nodeSize * sizeof(void));
    tempPointers = malloc((tree->nodeSize+1) * sizeof(void));
    
    //fills the keys and pointers
    for (i = 0, j = 0; i < parent->numOfKeys + 1; i++, j++) {
        if(j == insertPoint) j++;
        if(i<parent->numOfKeys){
          tempKeys[j] = parent->keys[i];}
        tempPointers[j] = parent->pointers[i];
    }
    
    tempKeys[leftIndex] = record;
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
    Record * newRecord = (Record *)newNode->keys[0];
    
    free(tempKeys);
    free(tempPointers);
    
    //old node to the left and new Node to the right
    return insertIntoParent(tree, parent, newRecord, rightNode);
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
Node * insertIntoNode(BPlusTree *tree, Node * parent, int leftIndex, Record *record, Node * newChild) {
   
    for (int i = parent->numOfKeys; i > leftIndex; i--) {
        
        //moves pointer one right till insertion point
        parent->pointers[i + 1] = parent->pointers[i];
        
        //because index is one too much (numOfKeys > index of last elemement)-> space for the new key
        parent->keys[i] = parent->keys[i - 1];
        
    }
    parent->pointers[leftIndex + 1] = newChild;
    parent->keys[leftIndex] = record;
    parent->numOfKeys++;
    
    return tree->root;
}

Node * insertIntoANewRoot(BPlusTree *tree, Node * left, Record *record, Node * right) {
    
    Node * root = Node_new(tree->nodeSize);
    root->keys[0] = record;
    root->pointers[0] = left;
    root->pointers[1] = right;
    root->numOfKeys = 1;
    root->parent = NULL;
    left->parent = root;
    right->parent = root;
    tree->root = root;
    return root;
}

void insertRecordIntoLeaf(Node *node, Record *record){
    
    int i, insertPoint;
    
    insertPoint = 0;
    while (insertPoint < node->numOfKeys && ((Record *)node->keys[insertPoint])->recordKey < record->recordKey){
        insertPoint++;
    }
   
    //updates Keys
    for (i = node->numOfKeys; i > insertPoint; i--) {
        //keys are moved one right to make space for new record
        node->keys[i] = node->keys[i - 1];
    }
    
    //no pointers necassary to update because node is directly inserted
    node->keys[insertPoint] = record;
    node->numOfKeys++;
    
    //debugging
    printf("\nElements in the node: ");
    for(int y = 0; y< node->numOfKeys; y++){
        printf("%fl ", ((Record *)node->keys[y])->recordKey);
    }
    
}

Node * findLeaf(BPlusTree *tree, double key){
    int i = 0;
    Node * c = tree->root;
    if (c == NULL) {
        return c;
    }
    while (!c->is_Leaf) {
        
        for (i = 0; i < c->numOfKeys - 1; i++){
            printf("%fl ", ((innerNode *)c->keys[i])->key);
        }
            i = 0;
            while (i < c->numOfKeys) {
                
                //key is bigger -> search again
                if (key >= ((innerNode *)c->keys[i])->key) i++;
                
                // key is smaller -> go to this pointer
                else break;
        }
            printf("%d ->\n", i);
        
            //new lookup node
            c = (Node *)c->pointers[i];
    }
    
    //debugging
    printf("Leaf [");
    for (i = 0; i < c->numOfKeys; i++){
     //       printf("%fl ", ((Record *)c->keys[i])->recordKey);
    }
    
    //leaf found
    return c;
}

void newTree(BPlusTree *tree, Record *record){
    tree->root = Node_new(tree->nodeSize);
    tree->root->is_Leaf = true;
    tree->root->keys[0] = record;
    tree->root->parent = NULL;
    tree->root->numOfKeys = 1;
}



/************************* CIRCULAR ARRAY ********************************************/

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
        if((newTime - (array->size - 1) * timestampDiff) <= newTime){
           minAllowedTime = newTime - (array->size - 1) *timestampDiff;
        }
        
        int positionStep = (int)(newTime - array->data[lastUpdatePosition].time)/timestampDiff;
        
        printf("last Update Pos was: %d\n", lastUpdatePosition);
        
        for (int i = 0; i<positionStep; i++) {
            
            double value = array->data[(lastUpdatePosition+i)%length].value;
            timestamp_t curTime = array->data[(lastUpdatePosition+i)%length].time;
            
            //checks if the time is in the range of the circular Array after the new value is inserted to it
            //Range is: newTime - (array->size-1) * timeDifference
            
            if(curTime < minAllowedTime && isfinite(value)){
                
                timestamp_t timeDelete = array->data[(lastUpdatePosition+i)%length].time;
                double valueDelete = array->data[(lastUpdatePosition+i)%length].value;
                
                //deletes the record from the array and the b+tree
                deleteRecordFromTree(tree, timeDelete, valueDelete);
                
        
                array->data[(lastUpdatePosition+i)%length].value = INFINITY;
                array->data[(lastUpdatePosition+i)%length].time = INFINITY;
            }
 
        }
        
        timestamp_t timeDelete = array->data[newUpdatePosition].time;
        double valueDelete = array->data[newUpdatePosition].value;
        
        if(isfinite(valueDelete)){
        //TODO
        //  deleteRecordFromTree(tree, timeDelete, valueDelete);
        }
        
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
