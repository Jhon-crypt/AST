/**
 * Test file with explicit hierarchy for testing
 */

#ifndef HIERARCHY_TEST_H
#define HIERARCHY_TEST_H

/**
 * @brief Top level struct
 */
typedef struct {
    int a;
    int b;
    
    /**
     * @brief Nested struct inside TopLevel
     */
    struct {
        int c;
        int d;
    } nested;
    
} TopLevel;

/**
 * @brief Function that uses the top level struct
 */
void process_top_level(TopLevel* top) {
    // Function body
}

#ifdef FEATURE_FLAG
/**
 * @brief Feature-specific struct
 */
typedef struct {
    int flag;
    
    /**
     * @brief Nested feature struct
     */
    struct {
        int x;
        int y;
    } feature_nested;
    
} FeatureStruct;

#endif /* FEATURE_FLAG */

#endif /* HIERARCHY_TEST_H */
