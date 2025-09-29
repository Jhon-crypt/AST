/**
 * Test file with very explicit nesting for hierarchy testing
 */

#ifndef EXPLICIT_NESTING_H
#define EXPLICIT_NESTING_H

/**
 * @brief Top level ifdef block
 */
#ifdef FEATURE_FLAG

/**
 * @brief Level 1 struct
 */
typedef struct Level1 {
    int a;
    int b;
    
    /**
     * @brief Level 2 struct
     */
    struct Level2 {
        int c;
        int d;
        
        /**
         * @brief Level 3 struct
         */
        struct Level3 {
            int e;
            int f;
        } level3;
        
    } level2;
    
} Level1;

#endif /* FEATURE_FLAG */

#endif /* EXPLICIT_NESTING_H */
