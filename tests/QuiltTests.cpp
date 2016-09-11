#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "../src/Quilt.h"

std::string *d1 = new std::string("0123456789");
Quilt q = QuiltSnippet(d1);

TEST(snippetTest, HandlesSimpleCreate) {
	EXPECT_EQ(q.GetCharOrFail(0), '0');
	EXPECT_EQ(q.GetCharOrFail(5), '5');
	EXPECT_EQ(q.GetCharOrFail(9), '9');
	EXPECT_THROW(q.GetCharOrFail(-1), NoDataHere);
	EXPECT_THROW(q.GetCharOrFail(10), NoDataHere);
}

TEST(snippetTest, HandlesNULLCreate) {
	EXPECT_THROW(QuiltSnippet(0), std::invalid_argument);
}

TEST(snippetTest, HandlesGetShortBE) {
	EXPECT_EQ(q.GetShortBEOrFail(0), 12337);
}

TEST(snippetTest, HandlesGetShortLE) {
	EXPECT_EQ(q.GetShortLEOrFail(0), 12592);
}

TEST(snippetTest, HandlesGetSubString) {
	EXPECT_STREQ(q.GetSubStringOrFail(0, 0)->c_str(), "");
	EXPECT_STREQ(q.GetSubStringOrFail(0, 1)->c_str(), "0");
	EXPECT_STREQ(q.GetSubStringOrFail(0, 3)->c_str(), "012");
	EXPECT_STREQ(q.GetSubStringOrFail(0, 10)->c_str(), "0123456789");
	EXPECT_STREQ(q.GetSubStringOrFail(5, 5)->c_str(), "56789");
	EXPECT_STREQ(q.GetSubStringOrFail(5, 3)->c_str(), "567");
	EXPECT_THROW(q.GetSubStringOrFail(0, 11), NoDataHere);
}

TEST(cutTest, HandlesNormalCut) {
	Quilt q_cut = QuiltCut(&q, 5, 10);
	EXPECT_EQ(q_cut.GetCharOrFail(0), '5');
	EXPECT_EQ(q_cut.GetCharOrFail(4), '9');
	EXPECT_THROW(q_cut.GetCharOrFail(10), NoDataHere);
}

TEST(cutTest, HandlesOutOfRangeCut) {
	Quilt q_cut = QuiltCut(&q, 200, 10);
	EXPECT_THROW(q_cut.GetCharOrFail(0), NoDataHere);
}

TEST(cutTest, HandlesSubString) {
	Quilt q_cut = QuiltCut(&q, 5, 10);

	EXPECT_STREQ(q_cut.GetSubStringOrFail(0, 0)->c_str(), "");
	EXPECT_STREQ(q_cut.GetSubStringOrFail(0, 1)->c_str(), "5");
	EXPECT_STREQ(q_cut.GetSubStringOrFail(0, 3)->c_str(), "567");
	EXPECT_STREQ(q_cut.GetSubStringOrFail(0, 5)->c_str(), "56789");
	EXPECT_STREQ(q_cut.GetSubStringOrFail(3, 2)->c_str(), "89");
	EXPECT_THROW(q_cut.GetSubStringOrFail(0, 11), NoDataHere);

}

TEST(sewTest, HandlesNormalSew) {
	Quilt q_cut1 = QuiltCut(&q, 5, 10);
	Quilt q_cut2 = QuiltCut(&q, 7, 10);
	QuiltSew q_sew = QuiltSew(20);
	q_sew.Sew(&q_cut1, 0);
	q_sew.Sew(&q_cut2, 10);

	EXPECT_EQ(q_sew.GetCharOrFail(0), '5');
	EXPECT_EQ(q_sew.GetCharOrFail(4), '9');
	EXPECT_THROW(q_sew.GetCharOrFail(5), NoDataHere);
	EXPECT_EQ(q_sew.GetCharOrFail(10), '7');
	EXPECT_EQ(q_sew.GetCharOrFail(12), '9');
	EXPECT_THROW(q_sew.GetCharOrFail(13), NoDataHere);
}

TEST(sewTest, HandlesGetSubString) {
	Quilt q_cut1 = QuiltCut(&q, 0, 5);
	Quilt q_cut2 = QuiltCut(&q, 5, 5);
	QuiltSew q_sew = QuiltSew(10);
	q_sew.Sew(&q_cut1, 0);
	q_sew.Sew(&q_cut2, 5);

	EXPECT_STREQ(q_sew.GetSubStringOrFail(0, 0)->c_str(), "");
	EXPECT_STREQ(q_sew.GetSubStringOrFail(0, 1)->c_str(), "0");
	EXPECT_STREQ(q_sew.GetSubStringOrFail(0, 3)->c_str(), "012");
	EXPECT_STREQ(q_sew.GetSubStringOrFail(1, 8)->c_str(), "12345678");
	EXPECT_STREQ(q_sew.GetSubStringOrFail(0, 10)->c_str(), "0123456789");
	EXPECT_STREQ(q_sew.GetSubStringOrFail(5, 5)->c_str(), "56789");
	EXPECT_STREQ(q_sew.GetSubStringOrFail(5, 3)->c_str(), "567");
	EXPECT_THROW(q_sew.GetSubStringOrFail(0, 11), NoDataHere);
	EXPECT_THROW(q_sew.GetSubStringOrFail(100, 11), NoDataHere);
}
