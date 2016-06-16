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
    serie * data;
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

typedef struct timeStampSet{
    int size;
    timestamp_t * timeStamps;
}timeStampSet;

timeStampSet * timeStampSet_new(int size){
    timeStampSet * newTimeStampSet = malloc(size * sizeof(timeStampSet));
    newTimeStampSet->size = size;
    return newTimeStampSet;
}

void timeStampSet_destroy(timeStampSet * timeStampSet){
    free(timeStampSet);
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

/* serie is element of circular array */
typedef struct neighborhood{
    listValue * posMinus;
    listValue * posPlus;
}neighborhood;

void neighborhood_destroy(neighborhood * neighborhood){
    free(neighborhood->posMinus);
    free(neighborhood->posPlus);
}

neighborhood * neighborhood_new(){
    neighborhood * neighboorhoodPointer = malloc(sizeof(neighborhood));
    neighboorhoodPointer->posMinus = NULL;
    neighboorhoodPointer->posPlus = NULL;
    
    return neighboorhoodPointer;
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

/*************************************** MASTER METHOD *******************************************/

void shift(BPlusTree *tree, CircularArray *array, timestamp_t time, double value);

/*************************************** CIRCULAR ARRAY *******************************************/
bool lookup(CircularArray *array, timestamp_t t, double *value);
void printArray(CircularArray * array, int arraySize);
void serie_update(BPlusTree *tree, CircularArray *array, timestamp_t t, double value);
void initialize_data(CircularArray *array);

/*************************************** NEIGHBOUR *******************************************/
timestamp_t neighbor(BPlusTree *tree, double value, timeStampSet * set);

void initialize_tree(BPlusTree *tree);

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

Node * findLeaf(BPlusTree *tree, double newKey);


/******** DELETION ********/
boolean isDuplicateKey(Node * curNode, timestamp_t newTime, double newKey);
void deleteFirstListValue(leafKey *leafKey);
leafKey * findLeafKeyAndSetBooleanIfMultipleListValues(Node * node, double key, boolean *hasMultipleTimes);
void delete(BPlusTree *tree, timestamp_t time, double value);
void deleteEntry(BPlusTree *tree, Node *node, double toDelete, Node * pointer);
Node * removeEntryFromTheNode(BPlusTree *tree, Node * node, double toDelete, Node * pointerNode);
void adjustTheRoot(BPlusTree *tree);

void mergeNodes(BPlusTree *tree, Node *node, Node *neighbor, int neighborIndex, void * kPrime);
void redestributeNodes(BPlusTree *tree, Node *node, Node *neighbor, int neighborIndex, int kIndex, void * pointer);

int getNeighbourIndex(Node * node);
void printTree(BPlusTree *tree);

/************** DUPLICATE HANDLING ****************/
void addDuplicateToDoublyLinkedList(Node *node, timestamp_t newTime, double duplicateKey);
int getInsertPoint(BPlusTree *tree, Node *oldNode, leafKey *leafKey);

/************** PRINT ****************/
void printLevelOrder(Node* root);
void addAndDeleteSomeExampleValuesFromTree(BPlusTree * tree);


/****************************** METHODS *******************************************************************/

int main(int argc, const char * argv[]) {
    
    //variable - can be commandLine Input
    int arraySize =3;
    int treeNodeSize = 4;
    int size = 5;
    timestamp_t searchedTime;
    timeStampSet * set = timeStampSet_new(size);
    
    CircularArray * array = CircularArray_new(arraySize);
    initialize_data(array);

    //create BplusTree;
    BPlusTree * tree = BPlusTree_new(treeNodeSize);
    addAndDeleteSomeExampleValuesFromTree(tree);
    
    double searchValue;
//    searchedTime = neighbor(tree, searchValue, set);

    BPlusTree_destroy(tree);
    CircularArray_destroy(array);

    return 0;
}

void shift(BPlusTree *tree, CircularArray *array, timestamp_t time, double value){
    
    //just new values are added to the tree and to the circular array
     addRecordToTree(tree, time, value);
     serie_update(tree, array, time, value);
    
}


/****************************** NEIGHBOUR METHOD *******************************************************************/

timestamp_t neighbor(BPlusTree *tree, double value, timeStampSet * set){
    
    boolean hasMultipleTimes = false;
    Node * leafNode;
    leafKey * leafKeyVal;
    neighborhood * newNeighborhood = neighborhood_new();
    
    leafNode = findLeaf(tree, value);
    leafKeyVal = findLeafKeyAndSetBooleanIfMultipleListValues(leafNode, value, &hasMultipleTimes);
    
    newNeighborhood->posMinus = leafKeyVal->firstListValue;

    if(hasMultipleTimes){
        newNeighborhood->posPlus = leafKeyVal->firstListValue->prev;
    }
    else{
        newNeighborhood->posPlus = leafKeyVal->firstListValue;
    }
        
    
    timestamp_t t = 0;
    
    
    return t;
}


/****************************** PRINT TREE *******************************************************************/

void addAndDeleteSomeExampleValuesFromTree(BPlusTree * tree){
    
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
    addRecordToTree(tree, 5100, 1600);
    addRecordToTree(tree, 5400, 1100);
    addRecordToTree(tree, 5700, 1100);
    
    
    printf("\n \n");
    
    
    printLevelOrder(tree->root);
    
    delete(tree, 3900, 2600);
    printf("\n \n");
    
    
    printLevelOrder(tree->root);
    delete(tree, 4500, 1400);
    printf("\n \n");
    
    
    printLevelOrder(tree->root);
    delete(tree, 4800, 1400);
    printf("\n \n");
    
    
    printLevelOrder(tree->root);
    delete(tree, 2400, 2100);
    printf("\n \n");
    
    
    printLevelOrder(tree->root);
    delete(tree, 5100, 1600);
    delete(tree, 1800, 1500);
    
    
    printf("\n \n");
    
    
    printLevelOrder(tree->root);
    

    
}

int height(Node * node){
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
    innerKey * innerKeyPrime;
    
    // Remove key and pointer from node.
    node = removeEntryFromTheNode(tree, node, toDelete, pointer);
    

    
    if (node == tree->root){
        adjustTheRoot(tree);
        return;
    }
    
    // deletion from a innernode or leaf
    if(node->is_Leaf){
        minNumberofKeys = getSplitPoint(tree->nodeSize);
    }
    else{
        minNumberofKeys = getSplitPoint(tree->nodeSize + 1) - 1;
    }
    
    //simple case - node has still enough keys
    if (node->numOfKeys >= minNumberofKeys){
        return;
    }
    
    /* Case:  node falls below minimum.
     * Either coalescence or redistribution
     * is needed.
     */
    
    /* Find the appropriate neighbor node with which to merge.
     * Also find the key (k_prime) in the parent
     * between the pointer to node n and the pointer
     * to the neighbor.
     */


    // Find the appropriate neighbor node with which to merge or redestribute.
    neighbourIndex = getNeighbourIndex(node);
    
    if(neighbourIndex ==-1){
        kIndex = 0;
    }
    else{
        kIndex = neighbourIndex;
    }
    
    innerKeyPrime = ((innerKey *)node->parent->keys[kIndex]);
  

    if(neighbourIndex == -1){
        neighbor = node->parent->pointers[1];
    }else{
        neighbor = node->parent->pointers[neighbourIndex];
    }
    
    if(node->is_Leaf){
        capacity = tree->nodeSize + 1;
    }
    else{
        capacity = tree->nodeSize;
    }
    capacity = tree->nodeSize;
    
    
    //Merge - both nodes together have enough space
    if ((neighbor->numOfKeys + node->numOfKeys ) < capacity){
        mergeNodes(tree, node, neighbor, neighbourIndex, innerKeyPrime);
    }
    
    //Redistribution
    else{
        redestributeNodes(tree, node, neighbor, neighbourIndex, kIndex, innerKeyPrime);
    }
}

void redestributeNodes(BPlusTree * tree, Node * node, Node * neighbor, int neighbourIndex, int kIndex, void * kPrime){
    
    int i;
    Node * tmp;
    
    /* Case: n has a neighbor to the left. Pull the neighbor's last key-pointer pair over
     * from the neighbor's right end to n's left end.
     */
    
    if (neighbourIndex != -1) {
        
        if (!node->is_Leaf){
            node->pointers[node->numOfKeys + 1] = node->pointers[node->numOfKeys];
        }
        
        for (i = node->numOfKeys; i > 0; i--) {
            node->keys[i] = node->keys[i - 1];
            if(!node->is_Leaf){
                node->pointers[i] = node->pointers[i - 1];
            }
        }
        
        if (!node->is_Leaf) {
            node->pointers[0] = neighbor->pointers[neighbor->numOfKeys];
            tmp = (Node *)node->pointers[0];
            tmp->parent = node;
            neighbor->pointers[neighbor->numOfKeys] = NULL;
            node->keys[0] = kPrime;
            node->parent->keys[kIndex] = neighbor->keys[neighbor->numOfKeys - 1];
            
        }
        else {

            node->keys[0] = neighbor->keys[neighbor->numOfKeys - 1];
            node->parent->keys[kIndex] = node->keys[0];
        }
    }
    
    /* Case: n is the leftmost child.
     * Take a key-pointer pair from the neighbor to the right.
     * Move the neighbor's leftmost key-pointer pair
     * to n's rightmost position.
     */
    
    else {
        if (node->is_Leaf) {
            node->keys[node->numOfKeys] = neighbor->keys[0];
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
            if(!node->is_Leaf){
                neighbor->pointers[i] = neighbor->pointers[i + 1];
            }
        }
        if (!node->is_Leaf){
            neighbor->pointers[i] = neighbor->pointers[i + 1];
        }
    }
    
    //n now has one more key and one more pointer the neighbor has one fewer
    node->numOfKeys++;
    neighbor->numOfKeys--;

}

void mergeNodes(BPlusTree *tree, Node *node, Node *neighbor, int neighborIndex, void * kPrime){
    
    int i, j, neighborInsertionIndex;
    Node * tmp;
    
    /* Swap neighbor with node if node is on the
     * extreme left and neighbor is to its right.
     */
    if (neighborIndex == -1) {
        tmp = node;
        node = neighbor;
        neighbor = tmp;
    }
    
    /* Starting point in the neighbor for copying
     * keys and pointers from the other node.
     * Recall that n and neighbor have swapped places
     * in the special case of the node being a leftmost child.
     */
    neighborInsertionIndex = neighbor->numOfKeys;
    
    /* Case:  nonleaf node.
     * Append k_prime and the following pointer.
     * Append all pointers and keys from the neighbor. */
    
    if (!node->is_Leaf) {
        
        //key
        neighbor->keys[neighborInsertionIndex] = kPrime;
        neighbor->numOfKeys++;
        
        for (i = neighborInsertionIndex + 1, j = 0; j < node->numOfKeys; i++, j++) {
            neighbor->keys[i] = node->keys[j];
            neighbor->pointers[i] = node->pointers[j];
            neighbor->numOfKeys++;
            node->numOfKeys--;
        }
        neighbor->pointers[i] = node->pointers[j];
        
        //All children must now point up to the same parent.
        for (i = 0; i < neighbor->numOfKeys + 1; i++) {
            tmp = (Node *)neighbor->pointers[i];
            tmp->parent = neighbor;
        }

    }
    
    /* In a leaf, append the keys and pointers of the node to the neighbor.
     * Set the neighbor's last pointer to point to what had been n's right neighbor.
     */
    else {
        for (i = neighborInsertionIndex, j = 0; j < node->numOfKeys; i++, j++) {
            neighbor->keys[i] = node->keys[j];
            neighbor->numOfKeys++;
        }
        
        //relink leafs
        if(node->next != NULL){
            neighbor->next = node->next;
            (node->next)->prev = neighbor;
            }
       }
    
    deleteEntry(tree, node->parent, ((innerKey *)kPrime)->key, node);
    
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
    if (!tree->root->is_Leaf) {
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

void delete(BPlusTree *tree, timestamp_t time, double value){
    
    Node * leaf;
    //default value for Multiple Times in Doubly linked List
    boolean hasMultipleTimes = false;
    
    leaf = findLeaf(tree, value);

    leafKey * leafKeyToDelete;
    
    leafKeyToDelete = findLeafKeyAndSetBooleanIfMultipleListValues(leaf, value, &hasMultipleTimes);
    
    if(leafKeyToDelete != NULL && leafKeyToDelete != NULL){
        
        //key has duplicates --> delete first value ind doubly linked list
        if(hasMultipleTimes){
            deleteFirstListValue(leafKeyToDelete);
            return;
        }
        
        deleteEntry(tree, leaf, leafKeyToDelete->leafKey, NULL);
        leafKey_destroy(leafKeyToDelete);
    }
    
    else{
        printf("ERROR - RECORD THAT SHOULD GET DELETED IS NOT IN TREE");
    }
}

Node * removeEntryFromTheNode(BPlusTree *tree, Node * node, double toDelete, Node * pointerNode){

    // Remove the key and shift other keys
    int i = 0;

    while (((innerKey *)node->keys[i])->key != toDelete){
            i++;
    }
    
    for (++i; i < node->numOfKeys; i++){
        node->keys[i - 1] = node->keys[i];
    }
    
    // Set the other pointers to NULL
    if (!node->is_Leaf){
        
        int y, numOfPointers;
        // Remove the pointer and shift other pointers accordingly.
        // First determine number of pointers.
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
    if (!node->is_Leaf){
        for (i = node->numOfKeys + 1; i < tree->nodeSize + 1; i++)
            node->pointers[i] = NULL;
    }
    
    
    return node;
}

leafKey * findLeafKeyAndSetBooleanIfMultipleListValues(Node * node, double key, boolean *hasMultipleTimes){
    
    int i;
    for(i = 0; i< node->numOfKeys; i++){
        double currentKey =((leafKey *)node->keys[i])->leafKey;
        
        if(key == currentKey){
            break;
        }
    }
    
    leafKey * currentLeafKey = ((leafKey *)node->keys[i]);
    
    if(currentLeafKey->leafKey == key){
        //first value in leaf
        listValue * firstListValue = currentLeafKey->firstListValue;
        
        if(firstListValue->next != NULL){
            *hasMultipleTimes = true;
            return currentLeafKey;
            
        }
        return currentLeafKey;
        
    }
    
    return NULL;
    
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


/************************************* COMMONLY USED METHODS **************************************/

Node * findLeaf(BPlusTree *tree, double newKey){
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
            if (newKey >= currentKey){
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

void addRecordToTree(BPlusTree *tree, timestamp_t time, double value){
    
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
    
    //leaves: treenodeSize - internal node: order
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
    
    //innerNode splitpoint
    split = getSplitPoint(tree->nodeSize + 1);
    
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
