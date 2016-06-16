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

/******************************TIMESTAMP DEF********************************************/

#define timestampDiff 300

//t = type
typedef unsigned long timestamp_t;

typedef bool boolean;
#define true 1
#define false 0

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
    newListValue->prev = NULL;
    newListValue->next = NULL;

    return newListValue;
}

typedef struct leafKey{
    double leafKey;
    struct listValue *firstListValue;
}leafKey;

leafKey *leafKey_new(timestamp_t time, double key) {
    
    leafKey *newLeafKey = NULL;
    newLeafKey = malloc(sizeof(leafKey));
    newLeafKey->leafKey = key;
    
    listValue * newListValue = listValue_new(time);
    newLeafKey->firstListValue = newListValue;
   
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
    void ** pointers;
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

innerKey * innerKey_new(double newKey){
    innerKey * newInnerKey = NULL;
    newInnerKey = malloc(sizeof(innerKey));
    newInnerKey->key = newKey;
    return newInnerKey;
}

void innerKey_destroy(innerKey * key){
    free(key);
}

Node * Node_new(int nodeSize){
    Node * node =NULL;
    node = malloc(sizeof(Node));
    node->keys = malloc(nodeSize * sizeof(void *));
    node->numOfKeys = 0;
    //always n+1 pointers
    node->is_Leaf = false;
    node->pointers = malloc((nodeSize + 1) * sizeof(void *));
    node->next = NULL;
    node->prev = NULL;
    return node;
}

Node *Leaf_new(int nodeSize){
    Node *node = Node_new(nodeSize);
    node->is_Leaf = true;
    node->pointers = NULL;
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

timestamp_t neighbour(timestamp_t t, double value);

void printArray(CircularArray * array, int arraySize);
void serie_update(BPlusTree *tree, CircularArray *array, timestamp_t t, double value);
void initialize_data(CircularArray *array);
void initialize_tree(BPlusTree *tree);


void deleteRecordFromTree(BPlusTree *tree, timestamp_t time, double value);

void addRecordToTree(BPlusTree *tree, timestamp_t t, double value);
void newTree(BPlusTree *tree, timestamp_t time, double value);
void insertRecordIntoLeaf(BPlusTree *tree, Node *leaf, leafKey *firstValue);
int getSplitPoint(int length);
void insertIntoParent(BPlusTree *tree, Node *oldChild, double newKey, Node *newChild);
void insertIntoANewRoot(BPlusTree *tree, Node * left, double value, Node * right);
void splitAndInsertIntoInnerNode(BPlusTree *tree, Node * parent, int leftIndex, double key, Node * rightNode);
int getLeftPointerPosition(Node * parent, Node * left);
void splitAndInsertIntoLeaves(BPlusTree *tree, Node *oldNode, leafKey *firstValue);
void insertIntoTheNode(Node * node, int index, double value, Node * newChild);


boolean isDuplicateKey(Node * curNode, timestamp_t newTime, double newKey);
void deleteFirstListValue(leafKey *leafKey);
leafKey * findLeafKeyAndSetBooleanIfMultipleListValues(Node * node, timestamp_t time, double key, boolean *hasMultipleTimes);

Record * findRecordandRecordLeaf(BPlusTree *tree, double value);
Node * deleteEntry(BPlusTree *tree, double value, Node *leaf);
Node * removeEntryFromTheNode(BPlusTree *tree, double value, Node * leaf);
Node * adjustTheRoot(BPlusTree *tree);
Node * mergeNodes(BPlusTree *tree, Node *node, Node *neighbor, int neighborIndex, double key);
Node * redestributeNodes(BPlusTree *tree, Node *node, Node *neighbor, int neighborIndex, int kIndex, double key);

Node * findLeaf(BPlusTree *tree, timestamp_t newTime, double newKey);


int getPointerIndex(Node * node);
void printTree(BPlusTree *tree);
void addDuplicateToDoublyLinkedList(Node *node, timestamp_t newTime, double duplicateKey);
int getInsertPoint(BPlusTree *tree, Node *oldNode, leafKey *leafKey);

void printLevelOrder(Node* root);



int main(int argc, const char * argv[]) {
    
    //variable - can be commandLine Input
    int arraySize =3;
    int treeNodeSize = 3;
    //timestamp_t ti = 300;
    
    CircularArray *array = CircularArray_new(arraySize);
    initialize_data(array);

    //create BplusTree;
    BPlusTree *tree = BPlusTree_new(treeNodeSize);
    
    
    addRecordToTree(tree, 300, 300);
    addRecordToTree(tree, 600, 300);
    addRecordToTree(tree, 900, 600);

    addRecordToTree(tree, 1200, 600);
    addRecordToTree(tree, 1500, 1200);

    addRecordToTree(tree, 1800, 1500);
    addRecordToTree(tree, 2100, 1800);
    
    addRecordToTree(tree, 2300, 900);
    addRecordToTree(tree, 2400, 2100);

    addRecordToTree(tree, 3300, 2400);
    addRecordToTree(tree, 3600, 2500);
   
    addRecordToTree(tree, 3900, 2600);
    printf("\n \n");
    
    
    printLevelOrder(tree->root);

    addRecordToTree(tree, 4200, 2700);
    addRecordToTree(tree, 4500, 1400);
    addRecordToTree(tree, 4800, 1400);


    
    printf("\n \n");
    
    
    printLevelOrder(tree->root);
    addRecordToTree(tree, 4800, 1600);
    addRecordToTree(tree, 5100, 1100);
    addRecordToTree(tree, 5500, 1100);


    printf("\n \n");
    
    
    printLevelOrder(tree->root);
    
    deleteRecordFromTree(tree, 3900, 2600);

    //TODO FIX INSERTION OF DUPLICATE AND INNER VALUE - probably shows to same pointer

    printf("\n \n");
 

    printLevelOrder(tree->root);

    
    BPlusTree_destroy(tree);
    CircularArray_destroy(array);

    return 0;
}

void shift(BPlusTree *tree, CircularArray *array, timestamp_t time, double value){
    
    //just new values are added to the tree and to the circular array
     addRecordToTree(tree, time, value);
     serie_update(tree, array, time, value);
    
}



/******************************PRINT TREE*******************************************************************/

int height(Node * node)
{
    if (node == NULL || node->pointers == NULL )
        return 1;
    else
    {
        /* compute the height of each subtree */
        int length = height(node->pointers[0]);
        return length + 1;
    }
}
    
/* Print nodes at a given level */
void printGivenLevel(Node * root, int level){
        if (root == NULL)
            return;

        if (level == 1){

            for(int i = 0; i < root->numOfKeys; i++){
                printf(" %.fl ", ((innerKey *)root->keys[i])->key);
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

/* Function to print level order traversal a tree*/
void printLevelOrder(Node* root){
        int h = height(root);
        int i;
        printf("\t \t \t ");
    for (i=1; i<=h; i++){
        printGivenLevel(root, i);

        printf("\n");
    }
}


/******************************DELETION*******************************************************************/


leafKey * findLeafKeyAndSetBooleanIfMultipleListValues(Node * node, timestamp_t time, double key, boolean *hasMultipleTimes){
    
    int i;
    for(i = 0; i< node->numOfKeys; i++){
        double currentKey =((leafKey *)node->keys[i])->leafKey;
        
        if(key == currentKey){
            break;
        }
    }
    
    leafKey * currentLeafKey = ((leafKey *)node->keys[i]);
    
    //first value in leaf
    listValue * firstListValue = currentLeafKey->firstListValue;
    
    if(firstListValue->next != NULL){
        *hasMultipleTimes = true;
        return currentLeafKey;
      
    }
    return currentLeafKey;
    
}

void deleteFirstListValue(leafKey *leafKey){
    
    //first value in leaf
    listValue * firstListValue = leafKey->firstListValue;
    
    listValue * next = firstListValue->next;
    listValue * prev = firstListValue->prev;
    
    //next is the second oldest key
    leafKey->firstListValue = next;
    prev->next = next;
    next->prev = prev;
    
    //just one time left
    if(prev->next == next->next){
        leafKey->firstListValue->next = NULL;
        leafKey->firstListValue->prev = NULL;
    }
    
    //always the first list value must be the oldest list value and
    // therefore the one that is deleted
    listValue_destroy(firstListValue);

}


void deleteRecordFromTree(BPlusTree *tree, timestamp_t time, double value){
    
    Node * leaf;
    //default value for Multiple Times in Doubly linked List
    boolean hasMultipleTimes = false;
    
    leaf = findLeaf(tree, time, value);
    
    leafKey * leafKeyToDelete;
    
    leafKeyToDelete = findLeafKeyAndSetBooleanIfMultipleListValues(leaf, time, value, &hasMultipleTimes);
    
    //key has duplicates --> delete first value ind doubly linked list
    if(hasMultipleTimes){
        deleteFirstListValue(leafKeyToDelete);
        return;
    }
    
    //TODO


}

/*
leafKey * findValueandLeaf(BPlusTree *tree, timestamp_t time, double value) {
    int i = 0;
    Node * leaf = findLeaf(tree, time, value);
    
    if (leaf == NULL) return NULL;
    
    for (i = 0; i < leaf->numOfKeys; i++){
        
        if (((Record *)leaf->keys[i])->recordKey == value) break;
    }
    if (i == leaf->numOfKeys)
        return NULL;
    else
        return (leafKey *)leaf->pointers[i];
    
    
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

/*
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

  */
/************************************* COMMONLY USED METHODS **************************************/

Node * findLeaf(BPlusTree *tree, timestamp_t newTime, double newKey){
    int i = 0;
    
    Node * curNode = tree->root;
    double currentKey;
    
    
    if (curNode == NULL) {
        return curNode;
    }
    
    while (!curNode->is_Leaf) {
        
        i = 0;
        
        while (i < curNode->numOfKeys) {
            
            currentKey = ((innerKey *)curNode->keys[i])->key;
            
            //key is bigger -> search again
            if (newKey > currentKey){
                i++;
            }
            
            //key is smaller -> go to this pointer
            else break;
        }
        
        //new lookup node
        curNode = (Node *)curNode->pointers[i];
    }
    
    //leaf found
    return curNode;
}

/************************************* INSERTION **************************************/

void addRecordToTree(BPlusTree *tree, timestamp_t time, double value){
    
    Node *leaf;

    //the tree does not exist yet --> create tree
    if (tree->root == NULL){
        newTree(tree, time, value);
        return;
    }
    
    //find the right leaf -- if duplicate: insertinto leaf
    leaf = findLeaf(tree, time, value);
    
    
    //if duplicate -> insert to leaf as Doubly linked list value
    if(isDuplicateKey(leaf, time, value)){
        addDuplicateToDoublyLinkedList(leaf, time, value);
        return;
    }
    
    leafKey *leafKeyValue = leafKey_new(time, value);
    
    // leaf has room for key and pointer
    if (leaf->numOfKeys < tree->nodeSize) {
        insertRecordIntoLeaf(tree, leaf, leafKeyValue);
        return;
    }
        
    //leaf must be split
    splitAndInsertIntoLeaves(tree, leaf, leafKeyValue);

 }

void addDuplicateToDoublyLinkedList(Node *node, timestamp_t newTime, double duplicateKey){
    
    int i;
    
    for(i = 0; i<node->numOfKeys; i++){
        double currentKey =((leafKey *)node->keys[i])->leafKey;
        
        if(duplicateKey == currentKey){
            break;
        }
    }
    
    listValue * newListValue = listValue_new(newTime);
    
    leafKey * currentLeafKey = ((leafKey *)node->keys[i]);
    
    //first value in leaf
    listValue * firstListValue = currentLeafKey->firstListValue;
    
    listValue * currentListValue = firstListValue;
    
    while (currentListValue->next != NULL && currentListValue->next->timestamp > currentListValue->timestamp) {
        currentListValue = currentListValue->next;
    }
    
    //update doubly linked pointers
    currentListValue->next = newListValue;
    
    newListValue->prev = currentListValue;
    newListValue->next = firstListValue;
    firstListValue->prev = newListValue;
}

boolean isDuplicateKey(Node * curNode, timestamp_t newTime, double newKey){
    
    /**duplicate**/
    // key is equal -> go to this pointer and directly insert to list
    for(int i = 0; i<curNode->numOfKeys; i++){
        double currentKey =((leafKey *)curNode->keys[i])->leafKey;
        
        if(newKey == currentKey){
            return true;
        }
    }
    return false;
    
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
    
    tempKeys = malloc(tree->nodeSize * sizeof(void *));
    
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
        nrOfTempKeys++;


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
   
    
    //fill second leaf
    for (j = 0, i = split; i < nrOfTempKeys; i++, j++) {
        newNode->keys[j] = tempKeys[i];
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
    keyForParent = ((leafKey *)newNode->keys[0])->leafKey;
    
    //free allocated memory of pointers
    free(tempKeys);
  
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

    innerKey * newInnerKey = innerKey_new(value);

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
    void **tempKeys;
    void **tempPointers;
    int  nrOfTempKeys = 0;

    newInnerNode = Node_new(tree->nodeSize);
    
    //tempKeys and pointer are temporarly filled with up to nodesize + 1 keys
    tempKeys = malloc((tree->nodeSize +1) * sizeof(void *));
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
    
    innerKey * newInnerKey = innerKey_new(key);
    
    tempKeys[index] = newInnerKey;
    tempPointers[index + 1] = childNode;
    nrOfTempKeys++;
    
    newInnerNode->numOfKeys = 0;
    oldInnerNode->numOfKeys = 0;
    
    split = getSplitPoint(tree->nodeSize);
    
    int x = 0;
    for (;x < split; x++) {
        oldInnerNode->keys[x] = tempKeys[x];
        oldInnerNode->pointers[x] = tempPointers[x];
        oldInnerNode->numOfKeys++;
    }
    oldInnerNode->pointers[x] = tempPointers[x];
    innerKey * leftMostKey = tempKeys[x];
    
    newInnerNode->parent = oldInnerNode->parent;

    newInnerNode->numOfKeys = nrOfTempKeys-oldInnerNode->numOfKeys-1;
    
    //one node is given to upper node and has not to be inserted in this one
    for (++x, j = 0; j < newInnerNode->numOfKeys; j++, x++) {
        //first key is not inserted to this node - it is inserted to upper node
        newInnerNode->pointers[j] = tempPointers[x];
        newInnerNode->keys[j] = tempKeys[x];
    }
    newInnerNode->pointers[j] = tempPointers[x];
    free(tempKeys);
    free(tempPointers);
    
    //set parent to new node
    for (int i = 0; i < newInnerNode->numOfKeys + 1; i++) {
        Node * childOfNewNode = newInnerNode->pointers[i];
        childOfNewNode->parent = newInnerNode;
    }
    

    
    //old node to the left and new Node to the right
    insertIntoParent(tree, oldInnerNode, leftMostKey->key, newInnerNode);
    

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
    innerKey * newInnerKey = innerKey_new(key);
    tree->root = root;
    root->keys[0] = newInnerKey;
    left->parent = root;
    right->parent = root;
    root->pointers[0] = left;
    root->pointers[1] = right;
    root->numOfKeys = 1;
    root->parent = NULL;
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
    
    
}

int getInsertPoint(BPlusTree *tree, Node *oldNode, leafKey *keyTimePair){
    int insertPoint = 0;
    
    while (oldNode->numOfKeys > insertPoint && ((leafKey *)oldNode->keys[insertPoint])->leafKey < keyTimePair->leafKey){
        insertPoint++;
    }
    
    return insertPoint;
}

//create a new Tree
void newTree(BPlusTree *tree, timestamp_t time, double value){
    tree->root = Leaf_new(tree->nodeSize);
    leafKey *lKey = leafKey_new(time, value);
    tree->root->keys[0] = lKey;
    tree->root->pointers = NULL;
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
