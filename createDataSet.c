//
//  createDataSet.c
//  BPlusTree
//
//  Created by Melina Mast on 23.07.16.
//  Copyright © 2016 Melina Mast. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <stddef.h>
#include "timeset.c"
#include <string.h>
#include "createDataSet.h"

#define TIMESTAMP_DIFF 300


int main(int argc, const char * argv[]) {
    
    FILE *fp;
    int i;
    
    fp = fopen("datenset.txt", "w");
    
    if(fp == NULL) {
        printf("Datei konnte nicht geoeffnet werden.\n");
    }else {
        unsigned long t = 0;
        srand(time(NULL));


        for(i=0; i<17520000; i++) {
            

            t += TIMESTAMP_DIFF;
            double rand_value = rand()% 10000;
            
            fprintf(fp, "%fl, %.2lul\n", rand_value, t);
        }
        printf("Zahlen wurden geschrieben.\n");
        fclose(fp);
    }

   
    return 0;
}
