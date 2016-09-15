
#include "Quilt.h"

void Quilt::CopyBytesOrFail(char *buffer, patch_position offset, patch_position size)
{
	if (0 == size) {
		return;
	}

	patch_position end_offset = offset + size;

	patch_position copied = 0;

	for (quilt::iterator it = Data.begin(); it != Data.end(); ++it) {
		patch_position end_patch_offset = (*it).Begin + (*it).Length;
		patch_position offset_in_patch = offset-(*it).Begin;
		if ((*it).Begin <= offset && offset < end_patch_offset) {
			// substring begins inside this patch
			if ((*it).Begin <= end_offset && end_offset < end_patch_offset) {
				// substring is just a part of this patch
				(*it).Data->copy(buffer, size, offset_in_patch+(*it).DataBegin);
				return;
			} else {
				patch_position sublen = (*it).Length-offset_in_patch;
				(*it).Data->copy(buffer, sublen, offset_in_patch+(*it).DataBegin);
				copied += sublen;
			}
		} else if (offset < (*it).Begin) {
			if (end_offset >= end_patch_offset) {
				// all bytes of this patch are part of substring
				(*it).Data->copy(buffer+(*it).Begin-offset, (*it).Length, (*it).DataBegin);
				copied += (*it).Length;
			} else {
				// substring ends in this patch
				patch_position sublen = end_offset-(*it).Begin;
				(*it).Data->copy(buffer+(*it).Begin-offset, sublen, (*it).DataBegin);
				copied += sublen;
			}
		}
	}

	if (copied != size) {
		throw NoDataHere();
	}
}

std::string *Quilt::GetSubStringOrFail(patch_position offset, patch_position size)
{
	char buf[size];

	CopyBytesOrFail(buf, offset, size);

	std::string *r = new std::string(buf, size);

	return (r);
}

const ternary::Ternary &Quilt::CompareChar(patch_position offset, const unsigned char with)
{
	Patch *p = GetPatch(offset);
	if (p) {
		if ((unsigned char)p->Data->at((offset-p->Begin)+p->DataBegin) == with) {
			return (ternary::True);
		} else {
			return (ternary::False);
		}
	}

	return (ternary::Unknown);
}

const ternary::Ternary &Quilt::CompareShortBE(patch_position offset, const unsigned short with) {
	Patch *p0 = GetPatch(offset);
	Patch *p1 = GetPatch(offset+1);
	using namespace ternary;
	if (!p0 || !p1) {
		return (Unknown);
	}

	if ((unsigned char)p0->Data->at((offset-p0->Begin)+p0->DataBegin) == with >> 8
			&& ((unsigned char)p1->Data->at((offset+1-p1->Begin)+p1->DataBegin) & 0xf) == (with & 0xf)) {
		return (True);
	} else {
		return (False);
	}
}

const ternary::Ternary &Quilt::CompareShortLE(patch_position offset, const unsigned short with) {
	Patch *p0 = GetPatch(offset);
	Patch *p1 = GetPatch(offset+1);
	using namespace ternary;
	if (!p0 || !p1) {
		return (Unknown);
	}

	if ((unsigned char)p1->Data->at((offset+1-p1->Begin)+p1->DataBegin) == with >> 8
			&& ((unsigned char)p0->Data->at((offset-p0->Begin)+p0->DataBegin) & 0xf) == (with & 0xf)) {
		return (True);
	} else {
		return (False);
	}
}

const ternary::Ternary &Quilt::CompareSubString(patch_position offset, const std::string &with)
{
	using namespace ternary;
	try {
		std::string *substr = GetSubStringOrFail(offset, with.length());
		if (*substr == with) {
			delete substr;
			return (True);
		} else {
			delete substr;
			return (False);
		}
	} catch (...) {
		return (Unknown);
	}
}

QuiltSnippet::QuiltSnippet(std::string *data)
{
	if (NULL == data) {
		throw std::invalid_argument("Cannot create QuiltSnippet from NULL");
	}

	Length = data->length();
	CoveredSize = data->length();

	Patch p = Patch(0, data->length(), data, 0);
	Data.push_back(p);
}

QuiltCut::QuiltCut(Quilt *origin, patch_position offset, patch_position length)
{
	Length = length;
	CoveredSize = 0;

	patch_position cut_endpos = offset+length;

	for (quilt::iterator it = origin->Data.begin(); it != origin->Data.end(); ++it) {
		patch_position patch_endpos = (*it).Begin + (*it).Length;

		if (offset < (*it).Begin) {
			// Cut begins before this patch
			if (cut_endpos > (*it).Begin) {
				if (cut_endpos < patch_endpos) {
					// Cut ends in this patch
					patch_position patch_length = cut_endpos-(*it).Begin;
					Patch p = Patch((*it).Begin, patch_length, (*it).Data, (*it).DataBegin);
					Data.push_back(p);
					CoveredSize += patch_length;
				} else {
					// This patch is in the middle of cut, copy it
					Data.push_back(*it);
					CoveredSize += (*it).Length;
				}
			} else {
				// Just skip this patch
			}
		} else {
			if (offset == (*it).Begin && cut_endpos == patch_endpos) {
				// This patch is equal to cut
				Data.push_back(*it);
				CoveredSize += (*it).Length;
			} else {
				if (offset < patch_endpos) {
					// Cut begins somewhere in this patch

					if (cut_endpos < patch_endpos) {
						// Cut is substring of this patch
						Patch p = Patch(0, length, (*it).Data, (offset-(*it).Begin)+(*it).DataBegin);
						Data.push_back(p);
						CoveredSize += length;
					} else {
						// Cut continues after this patch
						Patch p = Patch(0, patch_endpos-offset, (*it).Data, (offset-(*it).Begin)+(*it).DataBegin);
						Data.push_back(p);
						CoveredSize += patch_endpos-offset;
					}
				} else {
					// Cut begins after this patch, skip it
				}
			}
		}
	}

}

QuiltSew::QuiltSew(patch_position length)
{
	Length = length;
	CoveredSize = 0;
}

void QuiltSew::Sew(Quilt *origin, patch_position offset)
{
	for (quilt::iterator it = origin->Data.begin(); it != origin->Data.end(); ++it) {
		Patch p = (*it).Copy();
		p.Begin += offset;
		Data.push_back(p);
	}
	CoveredSize += origin->CoveredSize;
}
