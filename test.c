//
//  test.c
//  BPlusTree
//
//  Created by Melina Mast on 10.06.16.
//  Copyright © 2016 Melina Mast. All rights reserved.
//

#include "unity.h"
#include "IndexStructureTSD.h"



void test_AverageThreeBytes_should_AverageMidRangeValues(void)
{
    TEST_ASSERT_EQUAL_HEX(40, averageThreeBytes(30, 10, 0));
    
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_AverageThreeBytes_should_AverageMidRangeValues);
    return UNITY_END();
}