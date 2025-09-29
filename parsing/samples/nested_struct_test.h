/**
 * Test file with explicit nested structs for hierarchy testing
 */

#ifndef NESTED_STRUCT_TEST_H
#define NESTED_STRUCT_TEST_H

/**
 * @brief Outer struct
 */
typedef struct OuterStruct {
    int a;
    int b;
    
    /**
     * @brief Inner struct 1
     */
    struct InnerStruct1 {
        int c;
        int d;
        
        /**
         * @brief Deeply nested struct
         */
        struct DeepStruct {
            int e;
            int f;
        } deep;
        
    } inner1;
    
    /**
     * @brief Inner struct 2
     */
    struct InnerStruct2 {
        int g;
        int h;
    } inner2;
    
} OuterStruct;

#endif /* NESTED_STRUCT_TEST_H */
