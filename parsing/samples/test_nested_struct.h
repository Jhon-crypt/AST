/**
 * Test file for nested structures
 */

#ifndef TEST_NESTED_STRUCT_H
#define TEST_NESTED_STRUCT_H

typedef struct OuterStruct {
    int a;
    int b;
    
    struct MiddleStruct {
        int c;
        int d;
        
        struct InnerStruct {
            int e;
            int f;
        } inner;
        
    } middle;
    
} OuterStruct;

#endif /* TEST_NESTED_STRUCT_H */
