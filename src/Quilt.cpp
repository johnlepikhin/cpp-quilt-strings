
#include "Quilt.h"

#include <string>

PatchContent::PatchContent(std::shared_ptr<const std::string> content)
	: Content(content)
{
}

Quilt::Quilt()
{
	Length = 0;
	CoveredSize = 0;
	CachedPatch = NULL;
	CachedPatchBegin = 0xffffffff;
	CachedPatchEnd = 0xffffffff;
}

Quilt::Quilt(const patch_position length, const patch_position coveredSize)
	: Length(length)
	, CoveredSize(coveredSize)
	, CachedPatch(NULL)
	, CachedPatchBegin(0xffffffff)
	, CachedPatchEnd(0xffffffff)
{
}

void Quilt::AddNewPatch(
		const patch_position begin,
		const patch_position length,
		std::shared_ptr<PatchContent> data,
		const patch_position data_begin)
{
	std::shared_ptr<Patch> p(new Patch(begin, length, data, data_begin));
	AddPatch(p);
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
		std::shared_ptr<const std::string> data = (*it)->Data->Content;
		if ((*it)->Begin <= offset && offset < end_patch_offset) {
			// substring begins inside this patch
			if ((*it)->Begin <= end_offset && end_offset < end_patch_offset) {
				// substring is just a part of this patch
				data->copy(buffer, size, offset_in_patch+(*it)->DataBegin);
				return;
			} else {
				patch_position sublen = (*it)->Length-offset_in_patch;
				data->copy(buffer, sublen, offset_in_patch+(*it)->DataBegin);
				copied += sublen;
			}
		} else if (offset < (*it)->Begin) {
			if (end_offset >= end_patch_offset) {
				// all bytes of this patch are part of substring
				data->copy(buffer+(*it)->Begin-offset, (*it)->Length, (*it)->DataBegin);
				copied += (*it)->Length;
			} else {
				// substring ends in this patch
				patch_position sublen = end_offset-(*it)->Begin;
				data->copy(buffer+(*it)->Begin-offset, sublen, (*it)->DataBegin);
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
	try {
		r->resize(size);
		CopyBytesOrFail(const_cast<char *>(r->data()), offset, size);

		return (r);
	} catch (...) {
		delete r;
		throw;
	}
}

std::string *Quilt::GetMaxSubString(const patch_position offset, const patch_position size)
{
	std::string *r = new std::string();
	patch_position my_lastpos = offset;
	patch_position r_lastpos = 0;
	patch_position lastpos = offset+size;

	try {
		r->resize(size);

		while (my_lastpos <= lastpos) {
			const std::shared_ptr<Patch> p = GetPatch(my_lastpos);
			if (p) {
				patch_position data_begin = p->Begin - p->DataBegin + my_lastpos;
				patch_position offset_in_data = data_begin-p->DataBegin;
				patch_position copylen = p->Length-offset_in_data;
				if (r_lastpos+copylen > r->size()) {
					r->resize(r_lastpos+copylen);
				}
				p->Data->Content->copy(&(r->at(r_lastpos)), copylen, data_begin);
				my_lastpos+=copylen;
				r_lastpos+=copylen;
			} else {
				r->resize(r_lastpos);
				return (r);
			}
		}

		r->resize(r_lastpos);
		return (r);
	} catch (...) {
		std::cout << "Fail!!!\n";
		delete r;
		throw;
	}
}

const ternary::Ternary &Quilt::CompareChar(const patch_position offset, const unsigned char with)
{
	const std::shared_ptr<Patch> p = GetPatch(offset);
	if (p) {
		if ((unsigned char)p->Data->Content->at((offset-p->Begin)+p->DataBegin) == with) {
			return (ternary::True);
		} else {
			return (ternary::False);
		}
	}

	return (ternary::Unknown);
}

const ternary::Ternary &Quilt::CompareShortBE(const patch_position offset, const unsigned short with)
{
	const std::shared_ptr<Patch> p0 = GetPatch(offset);
	const std::shared_ptr<Patch> p1 = GetPatch(offset+1);
	using namespace ternary;
	if (!p0 || !p1) {
		return (Unknown);
	}

	if ((unsigned char)p0->Data->Content->at((offset-p0->Begin)+p0->DataBegin) == with >> 8
			&& ((unsigned char)p1->Data->Content->at((offset+1-p1->Begin)+p1->DataBegin) & 0xf) == (with & 0xf)) {
		return (True);
	} else {
		return (False);
	}
}

const ternary::Ternary &Quilt::CompareShortLE(const patch_position offset, const unsigned short with)
{
	const std::shared_ptr<Patch> p0 = GetPatch(offset);
	const std::shared_ptr<Patch> p1 = GetPatch(offset+1);
	using namespace ternary;
	if (!p0 || !p1) {
		return (Unknown);
	}

	if ((unsigned char)p1->Data->Content->at((offset+1-p1->Begin)+p1->DataBegin) == with >> 8
			&& ((unsigned char)p0->Data->Content->at((offset-p0->Begin)+p0->DataBegin) & 0xf) == (with & 0xf)) {
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

Quilt::Quilt(std::shared_ptr<const std::string> data)
	: Length(data->length())
	, CoveredSize(data->length())
	, CachedPatch(NULL)
	, CachedPatchBegin(0xffffffff)
	, CachedPatchEnd(0xffffffff)
{
	std::shared_ptr<PatchContent> content(new PatchContent(data));
	AddNewPatch(0, data->length(), content, 0);
}

Quilt::Quilt(std::shared_ptr<const std::string> data, const patch_position length)
	: Length(length)
	, CoveredSize(data->length())
	, CachedPatch(NULL)
	, CachedPatchBegin(0xffffffff)
	, CachedPatchEnd(0xffffffff)
{
	std::shared_ptr<PatchContent> content(new PatchContent(data));
	AddNewPatch(0, data->length(), content, 0);
}

void Quilt::Cut(std::shared_ptr<Quilt> origin, const patch_position offset, const patch_position length)
{
	patch_position cut_endpos = offset+length;

	for (quilt::const_iterator it = origin->Data.begin(); it != origin->Data.end(); ++it) {
		patch_position patch_endpos = (*it)->Begin + (*it)->Length;

		if (offset < (*it)->Begin) {
			// Cut begins before this patch
			if (cut_endpos > (*it)->Begin) {
				if (cut_endpos < patch_endpos) {
					// Cut ends in this patch
					patch_position patch_length = cut_endpos-(*it)->Begin;
					AddNewPatch((*it)->Begin, patch_length, (*it)->Data, (*it)->DataBegin);
					CoveredSize += patch_length;
				} else {
					// This patch is in the middle of cut, copy it
					AddPatch((*it)->Copy());
					CoveredSize += (*it)->Length;
				}
			} else {
				// Just skip this patch
			}
		} else {
			if (offset == (*it)->Begin && cut_endpos == patch_endpos) {
				// This patch is equal to cut
				AddPatch((*it)->Copy());
				CoveredSize += (*it)->Length;
			} else {
				if (offset < patch_endpos) {
					// Cut begins somewhere in this patch

					if (cut_endpos < patch_endpos) {
						// Cut is substring of this patch
						AddNewPatch(0, length, (*it)->Data, (offset-(*it)->Begin)+(*it)->DataBegin);
						CoveredSize += length;
					} else {
						// Cut continues after this patch
						AddNewPatch(0, patch_endpos-offset, (*it)->Data, (offset-(*it)->Begin)+(*it)->DataBegin);
						CoveredSize += patch_endpos-offset;
					}
				} else {
					// Cut begins after this patch, skip it
				}
			}
		}
	}
}

Quilt::Quilt(std::shared_ptr<Quilt> origin, const patch_position offset, const patch_position length)
	: Length(length)
	, CoveredSize(0)
	, CachedPatch(NULL)
	, CachedPatchBegin(0xffffffff)
	, CachedPatchEnd(0xffffffff)
{
	this->Cut(origin, offset, length);
}

Quilt::Quilt(std::shared_ptr<Quilt> origin, const patch_position offset)
	: Length(origin->CoveredSize-offset)
	, CoveredSize(0)
	, CachedPatch(NULL)
	, CachedPatchBegin(0xffffffff)
	, CachedPatchEnd(0xffffffff)
{
	patch_position length = origin->CoveredSize-offset;
	this->Cut(origin, offset, length);
}

Quilt::Quilt(const patch_position length)
	: Length(length)
	, CoveredSize(0)
	, CachedPatch(NULL)
	, CachedPatchBegin(0xffffffff)
	, CachedPatchEnd(0xffffffff)
{
}

void Quilt::Sew(std::shared_ptr<Quilt> origin, const patch_position offset, const bool resize)
{
	for (quilt::const_iterator it = origin->Data.begin(); it != origin->Data.end(); ++it) {
		AddNewPatch((*it)->Begin+offset, (*it)->Length, (*it)->Data, (*it)->DataBegin);
		const patch_position newLength = (*it)->Begin+offset+(*it)->Length;
		if (resize && newLength > Length) {
			Length = newLength;
		}
	}
	CoveredSize += origin->CoveredSize;
}

void Quilt::SewWithHole(std::shared_ptr<Quilt> origin, const patch_position offset, const patch_position length)
{
	for (quilt::const_iterator it = origin->Data.begin(); it != origin->Data.end(); ++it) {
		AddNewPatch((*it)->Begin+offset, (*it)->Length, (*it)->Data, (*it)->DataBegin);
		const patch_position newLength = (*it)->Begin+offset+length;
		if (newLength > Length) {
			Length = newLength;
		}
	}
	CoveredSize += origin->CoveredSize;
}
