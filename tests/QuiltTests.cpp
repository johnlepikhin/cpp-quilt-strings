#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>

#include "shared.h"

std::shared_ptr<const std::string> quilt_data1(new std::string("0123456789"));
std::shared_ptr<Quilt> quilt1(new Quilt(quilt_data1));

TEST(snippetTest, HandlesSimpleCreate) {
	EXPECT_EQ(quilt1->GetCharOrFail(0), '0');
	EXPECT_EQ(quilt1->GetCharOrFail(5), '5');
	EXPECT_EQ(quilt1->GetCharOrFail(9), '9');
	EXPECT_THROW(quilt1->GetCharOrFail(-1), NoDataHere);
	EXPECT_THROW(quilt1->GetCharOrFail(10), NoDataHere);
}

//TEST(snippetTest, HandlesNULLCreate) {
//	EXPECT_THROW(QuiltSnippet(0), std::invalid_argument);
//}

TEST(snippetTest, HandlesGetShortBE) {
	EXPECT_EQ(quilt1->GetShortBEOrFail(0), 12337);
}

TEST(snippetTest, HandlesGetShortLE) {
	EXPECT_EQ(quilt1->GetShortLEOrFail(0), 12592);
}

TEST(snippetTest, HandlesGetSubString) {
	std::string *r;

	r = quilt1->GetSubStringOrFail(0, 0);
	EXPECT_STREQ(r->c_str(), "");
	delete r;

	r = quilt1->GetSubStringOrFail(0, 1);
	EXPECT_STREQ(r->c_str(), "0");
	delete r;

	r = quilt1->GetSubStringOrFail(0, 3);
	EXPECT_STREQ(r->c_str(), "012");
	delete r;

	r = quilt1->GetSubStringOrFail(0, 10);
	EXPECT_STREQ(r->c_str(), "0123456789");
	delete r;

	r = quilt1->GetSubStringOrFail(5, 5);
	EXPECT_STREQ(r->c_str(), "56789");
	delete r;

	r = quilt1->GetSubStringOrFail(5, 3);
	EXPECT_STREQ(r->c_str(), "567");
	delete r;

	EXPECT_THROW(quilt1->GetSubStringOrFail(0, 11), NoDataHere);
}

TEST(cutTest, HandlesNormalCut) {
	std::shared_ptr<Quilt> q_cut(new Quilt(quilt1, 5, 10));
	EXPECT_EQ(q_cut->GetCharOrFail(0), '5');
	EXPECT_EQ(q_cut->GetCharOrFail(4), '9');
	EXPECT_THROW(q_cut->GetCharOrFail(10), NoDataHere);
}

TEST(cutTest, HandlesCutToEnd) {
	Quilt *q_cut = new Quilt(quilt1, 5);
	EXPECT_EQ(q_cut->GetCharOrFail(0), '5');
	EXPECT_EQ(q_cut->GetCharOrFail(4), '9');
	EXPECT_THROW(q_cut->GetCharOrFail(10), NoDataHere);

	delete q_cut;
}

TEST(cutTest, HandlesOutOfRangeCut) {
	Quilt q_cut = Quilt(quilt1, 200, 10);
	EXPECT_THROW(q_cut.GetCharOrFail(0), NoDataHere);
}

TEST(cutTest, HandlesSubString) {
	const Quilt &q_cut = Quilt(quilt1, 5, 10);

	std::string *r;

	r = q_cut.GetSubStringOrFail(0, 0);
	EXPECT_STREQ(r->c_str(), "");
	delete r;

	r = q_cut.GetSubStringOrFail(0, 1);
	EXPECT_STREQ(r->c_str(), "5");
	delete r;

	r = q_cut.GetSubStringOrFail(0, 3);
	EXPECT_STREQ(r->c_str(), "567");
	delete r;

	r = q_cut.GetSubStringOrFail(0, 5);
	EXPECT_STREQ(r->c_str(), "56789");
	delete r;

	r = q_cut.GetSubStringOrFail(3, 2);
	EXPECT_STREQ(r->c_str(), "89");
	delete r;

	EXPECT_THROW(q_cut.GetSubStringOrFail(0, 11), NoDataHere);
}

TEST(sewTest, HandlesNormalSew) {
	std::shared_ptr<Quilt> q_cut1(new Quilt(quilt1, 5, 10));
	std::shared_ptr<Quilt> q_cut2(new Quilt(quilt1, 7, 10));
	std::shared_ptr<Quilt> q_sew(new Quilt(20));
	q_sew->Sew(q_cut1, 0, false);
	q_sew->Sew(q_cut2, 10, false);

	EXPECT_EQ(q_sew->GetCharOrFail(0), '5');
	EXPECT_EQ(q_sew->GetCharOrFail(4), '9');
	EXPECT_THROW(q_sew->GetCharOrFail(5), NoDataHere);
	EXPECT_EQ(q_sew->GetCharOrFail(10), '7');
	EXPECT_EQ(q_sew->GetCharOrFail(12), '9');
	EXPECT_THROW(q_sew->GetCharOrFail(13), NoDataHere);
}

TEST(sewTest, HandlesResize) {
	std::shared_ptr<Quilt> q_cut1(new Quilt(quilt1, 5, 10));
	std::shared_ptr<Quilt> q_cut2(new Quilt(quilt1, 7, 10));
	std::shared_ptr<Quilt> q_sew(new Quilt((const patch_position)0));

	q_sew->Sew(q_cut1, 0, false);
	EXPECT_EQ(q_sew->Length, 0);

	q_sew->Sew(q_cut1, 0, true);
	EXPECT_EQ(q_sew->Length, 5);

	q_sew->SewWithHole(q_cut2, 10000, 10);
	EXPECT_EQ(q_sew->Length, 10010);
}

TEST(sewTest, HandlesGetSubString) {
	std::shared_ptr<Quilt> q_cut1(new Quilt(quilt1, 0, 5));
	std::shared_ptr<Quilt> q_cut2(new Quilt(quilt1, 5, 5));
	std::shared_ptr<Quilt> q_sew(new Quilt(10));
	q_sew->Sew(q_cut1, 0, false);
	q_sew->Sew(q_cut2, 5, false);

	std::string *r;

	r = q_sew->GetSubStringOrFail(0, 0);
	EXPECT_STREQ(r->c_str(), "");
	delete r;

	r = q_sew->GetSubStringOrFail(0, 1);
	EXPECT_STREQ(r->c_str(), "0");
	delete r;

	r = q_sew->GetSubStringOrFail(0, 3);
	EXPECT_STREQ(r->c_str(), "012");
	delete r;

	r = q_sew->GetSubStringOrFail(1, 8);
	EXPECT_STREQ(r->c_str(), "12345678");
	delete r;

	r = q_sew->GetSubStringOrFail(0, 10);
	EXPECT_STREQ(r->c_str(), "0123456789");
	delete r;

	r = q_sew->GetSubStringOrFail(5, 5);
	EXPECT_STREQ(r->c_str(), "56789");
	delete r;

	r = q_sew->GetSubStringOrFail(5, 3);
	EXPECT_STREQ(r->c_str(), "567");
	delete r;

	EXPECT_THROW(q_sew->GetSubStringOrFail(0, 11), NoDataHere);
	EXPECT_THROW(q_sew->GetSubStringOrFail(100, 11), NoDataHere);
}

TEST(quiltTest, HandlesCompareChar) {
	EXPECT_EQ(quilt1->CompareChar(0, '0'), ternary::True);
	EXPECT_EQ(quilt1->CompareChar(0, 'z'), ternary::False);
	EXPECT_EQ(quilt1->CompareChar(9, '9'), ternary::True);
	EXPECT_EQ(quilt1->CompareChar(9, 'z'), ternary::False);
	EXPECT_EQ(quilt1->CompareChar(1000, '9'), ternary::Unknown);
}

TEST(quiltTest, HandlesCompareShortBE) {
	EXPECT_EQ(quilt1->CompareShortBE(0, 12337), ternary::True);
	EXPECT_EQ(quilt1->CompareShortBE(8, 14393), ternary::True);
	EXPECT_EQ(quilt1->CompareShortBE(9, 14649), ternary::Unknown);
	EXPECT_EQ(quilt1->CompareShortBE(-1, 14393), ternary::Unknown);
	EXPECT_EQ(quilt1->CompareShortBE(1000, 0), ternary::Unknown);
}

TEST(quiltTest, HandlesCompareShortLE) {
	EXPECT_EQ(quilt1->CompareShortLE(0, 12592), ternary::True);
	EXPECT_EQ(quilt1->CompareShortLE(8, 14648), ternary::True);
	EXPECT_EQ(quilt1->CompareShortLE(9, 14649), ternary::Unknown);
	EXPECT_EQ(quilt1->CompareShortLE(-1, 12592), ternary::Unknown);
	EXPECT_EQ(quilt1->CompareShortLE(1000, 0), ternary::Unknown);
}

TEST(quiltTest, HandlesCompareSubString) {
	EXPECT_EQ(quilt1->CompareSubString(0, ""), ternary::True);
	EXPECT_EQ(quilt1->CompareSubString(0, "0"), ternary::True);
	EXPECT_EQ(quilt1->CompareSubString(0, "01"), ternary::True);
	EXPECT_EQ(quilt1->CompareSubString(0, "zz"), ternary::False);
	EXPECT_EQ(quilt1->CompareSubString(-1, ""), ternary::True);

	EXPECT_EQ(quilt1->CompareSubString(1, ""), ternary::True);
	EXPECT_EQ(quilt1->CompareSubString(1, "1"), ternary::True);
	EXPECT_EQ(quilt1->CompareSubString(1, "12"), ternary::True);
	EXPECT_EQ(quilt1->CompareSubString(1, "zz"), ternary::False);

	EXPECT_EQ(quilt1->CompareSubString(8, ""), ternary::True);
	EXPECT_EQ(quilt1->CompareSubString(8, "8"), ternary::True);
	EXPECT_EQ(quilt1->CompareSubString(8, "89"), ternary::True);
	EXPECT_EQ(quilt1->CompareSubString(8, "89z"), ternary::Unknown);
	EXPECT_EQ(quilt1->CompareSubString(8, "zz"), ternary::False);
}
