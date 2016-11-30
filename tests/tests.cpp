// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <gtest/gtest.h>
#include "shared.h"

int main(int argc, char **argv) {

	::testing::InitGoogleTest(&argc, argv);

	int r = RUN_ALL_TESTS();

	return (r);
}
