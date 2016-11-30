// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "../src/Ternary.h"

using namespace ternary;

TEST(ternaryTest, HandlesEq) {
	EXPECT_EQ(True, True);
	EXPECT_EQ(False, False);
}

TEST(ternaryTest, HandlesTernaryAnd) {
	EXPECT_EQ(True && True, True);
	EXPECT_EQ(True && False, False);
	EXPECT_EQ(True && Unknown, Unknown);
	EXPECT_EQ(False && True, False);
	EXPECT_EQ(False && False, False);
	EXPECT_EQ(False && Unknown, False);
	EXPECT_EQ(Unknown && True, Unknown);
	EXPECT_EQ(Unknown && False, False);
	EXPECT_EQ(Unknown && Unknown, Unknown);

	EXPECT_EQ(True && true, True);
	EXPECT_EQ(True && false, False);
	EXPECT_EQ(False && true, False);
	EXPECT_EQ(False && false, False);
	EXPECT_EQ(Unknown && true, Unknown);
	EXPECT_EQ(Unknown && false, False);
}

TEST(ternaryTest, HandlesTernaryOr) {
	EXPECT_EQ(True || True, True);
	EXPECT_EQ(True || False, True);
	EXPECT_EQ(True || Unknown, True);
	EXPECT_EQ(False || True, True);
	EXPECT_EQ(False || False, False);
	EXPECT_EQ(False || Unknown, Unknown);
	EXPECT_EQ(Unknown || True, True);
	EXPECT_EQ(Unknown || False, Unknown);
	EXPECT_EQ(Unknown || Unknown, Unknown);

	EXPECT_EQ(True || true, True);
	EXPECT_EQ(True || false, True);
	EXPECT_EQ(False || true, True);
	EXPECT_EQ(False || false, False);
	EXPECT_EQ(Unknown || true, True);
	EXPECT_EQ(Unknown || false, Unknown);
}

TEST(ternaryTest, HandlesToBoolOrFail) {
	EXPECT_EQ(True.BoolOrFail(), true);
	EXPECT_EQ(False.BoolOrFail(), false);
	EXPECT_THROW(Unknown.BoolOrFail(), UnknownValue);
}

TEST(ternaryTest, HandlesProbablyTrue) {
	EXPECT_EQ(True.ProbablyTrue(), true);
	EXPECT_EQ(False.ProbablyTrue(), false);
	EXPECT_EQ(Unknown.ProbablyTrue(), true);
}

TEST(ternaryTest, HandlesProbablyFalse) {
	EXPECT_EQ(True.ProbablyFalse(), true);
	EXPECT_EQ(False.ProbablyFalse(), false);
	EXPECT_EQ(Unknown.ProbablyFalse(), false);
}
