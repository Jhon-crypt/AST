/**
 * Test file for include guard container
 */

#ifndef TEST_INCLUDE_GUARD_H
#define TEST_INCLUDE_GUARD_H

// This should be inside the include guard
typedef struct {
    int a;
    int b;
} SimpleStruct;

// This should be inside the include guard too
enum TestEnum {
    VALUE_1,
    VALUE_2,
    VALUE_3
};

#endif /* TEST_INCLUDE_GUARD_H */