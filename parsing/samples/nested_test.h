/**
 * Test file with nested structures for hierarchy testing
 */

#ifndef TEST_NESTED_H
#define TEST_NESTED_H

/**
 * @brief Top level documentation
 */
#ifdef CONFIG_FEATURE_A
/**
 * @brief Feature A documentation
 */
typedef struct {
    int a;
    int b;
} FeatureA;

#ifdef CONFIG_FEATURE_A_EXTENDED
/**
 * @brief Extended feature A
 */
typedef struct {
    FeatureA base;
    int extension;
} FeatureAExtended;
#endif /* CONFIG_FEATURE_A_EXTENDED */

#endif /* CONFIG_FEATURE_A */

#ifdef CONFIG_FEATURE_B
/**
 * @brief Feature B documentation
 */
typedef struct {
    int x;
    int y;
} FeatureB;
#endif /* CONFIG_FEATURE_B */

#endif /* TEST_NESTED_H */
