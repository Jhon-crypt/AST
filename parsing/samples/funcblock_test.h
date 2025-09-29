/**
 * Test file with FUNCBLOCK pattern for hierarchy testing
 */

#ifndef FUNCBLOCK_TEST_H
#define FUNCBLOCK_TEST_H

/**
 * @brief Top level documentation
 */
#ifdef __FUNCBLOCK__
/**
 * @brief FUNCBLOCK documentation
 */
#define FUNCBLOCK_DEFINED

typedef struct {
    int warning;
    int error;
} TBlockErrorState;

typedef struct {
    int warning;
    int error;
    int stopFunc;
} TInputErrorState;

#endif /* __FUNCBLOCK__ */

#endif /* FUNCBLOCK_TEST_H */
