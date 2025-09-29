/**
 * Simple nesting test
 */

#define SIMPLE_NESTING_H

/**
 * @brief Outer struct
 */
struct OuterStruct {
    int a;
    
    /**
     * @brief Inner struct
     */
    struct InnerStruct {
        int b;
    };
};
