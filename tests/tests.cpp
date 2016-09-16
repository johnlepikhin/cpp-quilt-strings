#include <gtest/gtest.h>
#include "shared.h"

int main(int argc, char **argv) {
	quilt_data1 = new std::string("0123456789");
	quilt1 = new QuiltSnippet(quilt_data1);

	::testing::InitGoogleTest(&argc, argv);

	int r = RUN_ALL_TESTS();

	delete quilt1;

	return (r);
}
