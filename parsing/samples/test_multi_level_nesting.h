/**
 * Test file with multiple levels of nesting
 */

#ifndef TEST_MULTI_LEVEL_NESTING_H
#define TEST_MULTI_LEVEL_NESTING_H

typedef struct OuterStruct {
    int a;
    int b;
    
    struct MiddleStruct1 {
        int c;
        int d;
        
        struct InnerStruct1 {
            int e;
            int f;
            
            struct DeepStruct1 {
                int g;
                int h;
            } deep1;
            
        } inner1;
        
    } middle1;
    
    struct MiddleStruct2 {
        int i;
        int j;
        
        struct InnerStruct2 {
            int k;
            int l;
        } inner2;
        
    } middle2;
    
} OuterStruct;

#endif /* TEST_MULTI_LEVEL_NESTING_H */
