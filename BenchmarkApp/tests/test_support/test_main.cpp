#include "test_support/assert.h"

int main() {
    test_support::expect_true(true, "smoke");
    return test_support::finish();
}
