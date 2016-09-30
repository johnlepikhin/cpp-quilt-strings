#include <gtest/gtest.h>
#include "shared.h"

int main(int argc, char **argv) {

	::testing::InitGoogleTest(&argc, argv);

	int r = RUN_ALL_TESTS();

	return (r);
}
