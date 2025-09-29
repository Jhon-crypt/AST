/**
 * Test file for FUNCBLOCK depth calculation
 */

#ifndef TEST_FUNCBLOCK_DEPTH_H
#define TEST_FUNCBLOCK_DEPTH_H

#define __FUNCBLOCK__

typedef struct {
    int warning;
    int error;
} TBlockErrorState;

typedef struct {
    int warning;
    int error;
    int stopFunc;
} TInputErrorState;

#endif /* TEST_FUNCBLOCK_DEPTH_H */
