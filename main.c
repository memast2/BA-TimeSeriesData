//
//  main.c
//  BPlusTree
//  Created by Melina Mast on 20.03.16.
//


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>


//maximum number of records:The minimum number of keys per record is 1/2 of the maximum number of keys. For example, if the order of a B+ tree is n, each node (except for the root) must have between n/2 and n keys.For a n-order B+ tree with a height of h:
//maximum number of keys is n^h
//minimum number of keys is 2(n/2)^{h-1}.

//TODO BEDEUTUNG NODESIZE VS DEGREE KLÄREN
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
}Node;

typedef struct innerKey{
    double key;
    //does the nodes contain the pointers or the keys?
    void ** pointer;
}innerNode;


Node *Node_new(int nodeSize){
    Node *node = NULL;
    node = malloc(sizeof(Node));
    node->keys = malloc(nodeSize * sizeof(void));
    //always n+1 pointers
    node->pointers = malloc((nodeSize + 1) * sizeof(void));
    //FRAGE: SOLL MAN SO ZEIGEN DASS DER POINTER NIRGENDSWO HINZEIGT??
    node->pointers = NULL;
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
Node * findLeaf(BPlusTree *tree, Record *record);
void insertRecordIntoLeaf(Node *node, Record *record);
Node * splitAndInsert(BPlusTree *tree, Node *node, Record *record);
int getSplitPoint(int length);
Node * insertIntoParent(BPlusTree *tree, Node *oldLeaf, Record *record, Node *newLeaf);
Node * insertIntoANewRoot(BPlusTree *tree, Node * left, Record *record, Node * right);
Node * insertIntoNode(BPlusTree *tree, Node * n, int leftIndex, Record *record, Node * right);
int getLeftIndex(Node * parent, Node * left);


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

 
    
    //test with out of order timestamp : Annahme ankommender Timestamp ist grösser als alle zuvor
  //  shift(tree, array, 1200, 1200);
   // shift(tree, array, 2400, 2400);
 //   shift(tree, array, 3000, 3000);
    
    
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
    
   // BPlusTree_destroy(tree);
    CircularArray_destroy(array);

    return 0;
}

void shift(BPlusTree *tree, CircularArray *array, timestamp_t time, double value){
    
   // addRecordToTree(tree, time, value);
    serie_update(tree, array, time, value);

}

//Insertion
//Perform a search to determine which leaf node the new key should go into.
//If the node is not full, insert the new key, done!
//Otherwise, split the leaf node.
//Allocate a new leaf node and move half keys to the new node.
//Insert the new leaf's smallest key into the parent node.
//If the parent is full, split it too, repeat the split process above until a parent is found that need not split.
//If the root splits, create a new root which has one key and two children.
void addRecordToTree(BPlusTree *tree, timestamp_t time, double value){
    
    Record *record = record_new(time, value);
    Node *leaf;

    /* Case: the tree does not exist yet*/
    if (tree->root == NULL){
       return newTree(tree, record);
    }
    
    /* Case: the tree already exists.
     * (Rest of function body.)
     */
    leaf = findLeaf(tree, record);
    
    /* Case: leaf has room for key and pointer.
     */
    if (leaf->numOfKeys < tree->nodeSize) {
        return insertRecordIntoLeaf(leaf, record);
    }
    
    /* Case: leaf must be split.
     */
    splitAndInsert(tree, leaf, record);
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

Node * splitAndInsert(BPlusTree *tree, Node *node, Record *record){
    
    Node * newLeaf;
    int insertPoint, split, i, j;
    void **tempKeys;
    void **tempPointers;
    
    newLeaf = Leaf_new(tree->nodeSize);
    
    tempKeys = malloc(tree->nodeSize * sizeof(void));
    tempPointers = malloc((tree->nodeSize+1) * sizeof(void));
    
    insertPoint = 0;
    //<=because of possible duplicates
    while (insertPoint < tree->nodeSize && ((Record *)node->keys[insertPoint])->recordKey <= record->recordKey){
        insertPoint++;}
    
    
    //fills the keys and pointers
    for (i = 0, j = 0; i < node->numOfKeys; i++, j++) {
        if (j == insertPoint) j++;
        tempKeys[j] = node->keys[i];
        //pointer from first node are now pointers from the new one
      //  tempPointers[j] = node->pointers[i];

    }
    
    tempKeys[insertPoint] = record;
    tempPointers[j] = record;

    newLeaf->numOfKeys = 0;
    split = getSplitPoint(tree->nodeSize);
    
    for (i = 0; i < split; i++) {
        node->keys[i] = tempKeys[i];
        node->pointers[i] = tempPointers[i];
        node->numOfKeys++;
    }
    
    for (i = split, j = 0; i < tree->nodeSize; i++, j++) {
        newLeaf->keys[j] = tempKeys[i];
        newLeaf->pointers[i] = tempPointers[i];
        newLeaf->numOfKeys++;
    }
    //TODO: how to handle the pointes
    free(tempKeys);
    free(tempPointers);
    
    newLeaf->pointers[tree->nodeSize+1] = node->pointers[tree->nodeSize+1];
    node->pointers[tree->nodeSize+1] = newLeaf;
    
    for (i = node->numOfKeys; i < tree->nodeSize; i++)
        node->pointers[i] = NULL;
    for (i = newLeaf->numOfKeys; i < tree->nodeSize; i++)
        newLeaf->pointers[i] = NULL;
    
    newLeaf->parent = node->parent;
    Record * newRecord = (Record *)newLeaf->keys[0];
    
    return insertIntoParent(tree, node, newRecord, newLeaf);
}

Node * insertIntoParent(BPlusTree *tree, Node *oldLeaf, Record *newRecord, Node *newLeaf){
    
    int leftIndex;
    Node *parent;
    
    parent = oldLeaf->parent;
    
    /* Case: new root*/
    if (parent == NULL)
        return insertIntoANewRoot(tree, oldLeaf, newRecord, newLeaf);
    
    /* Case: leaf or node*/
    /* Find the parent's pointer from the old node.*/
    
    leftIndex = getLeftIndex(parent, oldLeaf);
    
    
    /* Case: the new key fits into the node*/
    
    if (parent->numOfKeys < tree->nodeSize)
        return insertIntoNode(tree, parent, leftIndex, newRecord, newLeaf);
    
    /* Case:  split a node in order to preserve the B+ tree properties*/
    return splitAndInsert(tree, parent, newRecord);

}


/* used in insertIntoParent to find the index of the parent's pointer to
 * the node to the left of the key to be inserted.
 */
int getLeftIndex(Node * parent, Node * left){
    
    int leftIndex = 0;
    
    while (leftIndex <= parent->numOfKeys &&
           parent->pointers[leftIndex] != left){
        leftIndex++;
    }
        
    return leftIndex;
}

/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 */
Node * insertIntoNode(BPlusTree *tree, Node * n,
                        int leftIndex, Record *record, Node * right) {
    int i;
    
    for (i = n->numOfKeys; i > leftIndex; i--) {
        n->pointers[i + 1] = n->pointers[i];
        n->keys[i] = n->keys[i - 1];
    }
    n->pointers[leftIndex + 1] = right;
    n->keys[leftIndex] = record;
    n->numOfKeys++;
    return tree->root;
}

Node * insertIntoANewRoot(BPlusTree *tree, Node * left, Record *record, Node * right) {
    
    Node * root = Node_new(tree->nodeSize);
    root->keys[0] = record;
    //kann theoretisch auch mehr pointer haben, hier ist es aber eine neue Root
    root->pointers[0] = left;
    root->pointers[1] = right;
    root->numOfKeys++;
    root->parent = NULL;
    left->parent = root;
    right->parent = root;
    return root;
}

void insertRecordIntoLeaf(Node *node, Record *record){
    
    int i, insertPoint;
    
    insertPoint = 0;
    while (insertPoint < node->numOfKeys && ((Record *)node->keys[insertPoint])->recordKey <= record->recordKey){
        insertPoint++;
    }

    
    //TODO: POINTER HANDLING
    //updates Pointers
    for (i = node->numOfKeys; i > insertPoint; i--) {
        //node->keys[i] = node->keys[i - 1];
        node->pointers[i] = node->pointers[i - 1];
    }
    node->keys[insertPoint] = record;
    //node->pointers[insertPoint] = pointer;
    node->numOfKeys++;
    
    //debugging
    printf("\nElements in the node: ");
    for(int y = 0; y<node->numOfKeys; y++){
        printf("%fl ", ((Record *)node->keys[y])->recordKey);
    }
    
}

Node * findLeaf(BPlusTree *tree, Record *record){
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
                //key is bigger or equal to the already added keys in c
                if (record->recordKey >= ((innerNode *)c->keys[i])->key) i++;
                else break;
        }
            printf("%d ->\n", i);
            c = (Node *)c->pointers[i];
    }
    
    //debugging
    printf("Leaf [");
    for (i = 0; i < c->numOfKeys; i++){
            printf("%fl ", ((Record *)c->keys[i])->recordKey);}
    
    //leaf found
    return c;
}

void deleteRecordFromTree(BPlusTree *tree, timestamp_t time, double value){
    
}


void searchTree(BPlusTree *tree, timestamp_t time, double value){
    
}

void newTree(BPlusTree *tree, Record *record){
    tree->root = Node_new(tree->nodeSize);
    tree->root->is_Leaf = true;
    tree->root->keys[0] = record;
    tree->root->parent = NULL;
    tree->root->numOfKeys = 1;
}



/*************************CIRCULAR ARRAY********************/


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
            deleteRecordFromTree(tree, timeDelete, valueDelete);
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
