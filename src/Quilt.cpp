
#include "Quilt.h"

Quilt::Quilt()
{
	Length = 0;
	CoveredSize = 0;
}

Quilt::Quilt(const patch_position length, const patch_position coveredSize)
	: Length(length)
	, CoveredSize(coveredSize)
{

}

void Quilt::CopyBytesOrFail(char *buffer, patch_position offset, patch_position size) const
{
	if (0 == size) {
		return;
	}

	patch_position end_offset = offset + size;

	patch_position copied = 0;

	for (quilt::const_iterator it = Data.begin(); it != Data.end(); ++it) {
		patch_position end_patch_offset = (*it)->Begin + (*it)->Length;
		patch_position offset_in_patch = offset-(*it)->Begin;
		if ((*it)->Begin <= offset && offset < end_patch_offset) {
			// substring begins inside this patch
			if ((*it)->Begin <= end_offset && end_offset < end_patch_offset) {
				// substring is just a part of this patch
				(*it)->Data.copy(buffer, size, offset_in_patch+(*it)->DataBegin);
				return;
			} else {
				patch_position sublen = (*it)->Length-offset_in_patch;
				(*it)->Data.copy(buffer, sublen, offset_in_patch+(*it)->DataBegin);
				copied += sublen;
			}
		} else if (offset < (*it)->Begin) {
			if (end_offset >= end_patch_offset) {
				// all bytes of this patch are part of substring
				(*it)->Data.copy(buffer+(*it)->Begin-offset, (*it)->Length, (*it)->DataBegin);
				copied += (*it)->Length;
			} else {
				// substring ends in this patch
				patch_position sublen = end_offset-(*it)->Begin;
				(*it)->Data.copy(buffer+(*it)->Begin-offset, sublen, (*it)->DataBegin);
				copied += sublen;
			}
		}
	}

	if (copied != size) {
		throw NoDataHere();
	}
}

std::string *Quilt::GetSubStringOrFail(const patch_position offset, const patch_position size) const
{
	std::string *r = new std::string();
	r->resize(size);
	CopyBytesOrFail((char *)r->data(), offset, size);

	return (r);
}

const ternary::Ternary &Quilt::CompareChar(const patch_position offset, const unsigned char with) const
{
	const Patch *p = GetPatch(offset);
	if (p) {
		if ((unsigned char)p->Data.at((offset-p->Begin)+p->DataBegin) == with) {
			return (ternary::True);
		} else {
			return (ternary::False);
		}
	}

	return (ternary::Unknown);
}

const ternary::Ternary &Quilt::CompareShortBE(const patch_position offset, const unsigned short with) const
{
	const Patch *p0 = GetPatch(offset);
	const Patch *p1 = GetPatch(offset+1);
	using namespace ternary;
	if (!p0 || !p1) {
		return (Unknown);
	}

	if ((unsigned char)p0->Data.at((offset-p0->Begin)+p0->DataBegin) == with >> 8
			&& ((unsigned char)p1->Data.at((offset+1-p1->Begin)+p1->DataBegin) & 0xf) == (with & 0xf)) {
		return (True);
	} else {
		return (False);
	}
}

const ternary::Ternary &Quilt::CompareShortLE(const patch_position offset, const unsigned short with) const
{
	const Patch *p0 = GetPatch(offset);
	const Patch *p1 = GetPatch(offset+1);
	using namespace ternary;
	if (!p0 || !p1) {
		return (Unknown);
	}

	if ((unsigned char)p1->Data.at((offset+1-p1->Begin)+p1->DataBegin) == with >> 8
			&& ((unsigned char)p0->Data.at((offset-p0->Begin)+p0->DataBegin) & 0xf) == (with & 0xf)) {
		return (True);
	} else {
		return (False);
	}
}

const ternary::Ternary &Quilt::CompareSubString(patch_position offset, const std::string &with) const
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

QuiltSnippet::QuiltSnippet(const std::string &data)
	: Quilt(data.length(), data.length())
{
	const Patch *p = new Patch(0, data.length(), data, 0);
	Data.push_back(p);
}

void QuiltCut::Cut(const Quilt &origin, const patch_position offset, const patch_position length)
{
	patch_position cut_endpos = offset+length;

	for (quilt::const_iterator it = origin.Data.begin(); it != origin.Data.end(); ++it) {
		patch_position patch_endpos = (*it)->Begin + (*it)->Length;

		if (offset < (*it)->Begin) {
			// Cut begins before this patch
			if (cut_endpos > (*it)->Begin) {
				if (cut_endpos < patch_endpos) {
					// Cut ends in this patch
					patch_position patch_length = cut_endpos-(*it)->Begin;
					const Patch *p = new Patch((*it)->Begin, patch_length, (*it)->Data, (*it)->DataBegin);
					Data.push_back(p);
					CoveredSize += patch_length;
				} else {
					// This patch is in the middle of cut, copy it
					Data.push_back(*it);
					CoveredSize += (*it)->Length;
				}
			} else {
				// Just skip this patch
			}
		} else {
			if (offset == (*it)->Begin && cut_endpos == patch_endpos) {
				// This patch is equal to cut
				Data.push_back(*it);
				CoveredSize += (*it)->Length;
			} else {
				if (offset < patch_endpos) {
					// Cut begins somewhere in this patch

					if (cut_endpos < patch_endpos) {
						// Cut is substring of this patch
						const Patch *p = new Patch(0, length, (*it)->Data, (offset-(*it)->Begin)+(*it)->DataBegin);
						Data.push_back(p);
						CoveredSize += length;
					} else {
						// Cut continues after this patch
						const Patch *p = new Patch(0, patch_endpos-offset, (*it)->Data, (offset-(*it)->Begin)+(*it)->DataBegin);
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

QuiltCut::QuiltCut(const Quilt &origin, const patch_position offset, const patch_position length)
	: Quilt(length, 0)
{
	this->Cut(origin, offset, length);
}

QuiltCut::QuiltCut(const Quilt &origin, const patch_position offset)
{
	patch_position length = origin.CoveredSize-offset;
	this->Cut(origin, offset, length);
}

QuiltSew::QuiltSew(const patch_position length)
{
	Length = length;
	CoveredSize = 0;
}

void QuiltSew::Sew(const Quilt &origin, const patch_position offset)
{
	for (quilt::const_iterator it = origin.Data.begin(); it != origin.Data.end(); ++it) {
		const Patch *p = new Patch((*it)->Begin+offset, (*it)->Length, (*it)->Data, (*it)->DataBegin);
		Data.push_back(p);
	}
	CoveredSize += origin.CoveredSize;
}
