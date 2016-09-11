
#include "Quilt.h"

std::string *Quilt::GetSubStringOrFail(patch_position offset, patch_position size)
{
	std::string *r = new std::string();
	r->resize(size);
	patch_position end_offset = offset + size;

	patch_position copied = 0;

	if (0 == size) {
		return (r);
	}

	for (quilt::iterator it = Data.begin(); it != Data.end(); ++it) {
		patch_position end_patch_offset = (*it).Begin + (*it).Length;
		patch_position offset_in_patch = offset-(*it).Begin;
		if ((*it).Begin <= offset && offset < end_patch_offset) {
			// substring begins inside this patch
			if ((*it).Begin <= end_offset && end_offset < end_patch_offset) {
				// substring is just a part of this patch
				r->replace(0, size, *((*it).Data), offset_in_patch+(*it).DataBegin, size);
				return (r);
			} else {
				patch_position sublen = (*it).Length-offset_in_patch;
				r->replace(0, sublen, *((*it).Data), offset_in_patch+(*it).DataBegin, sublen);
				copied += sublen;
			}
		} else if (offset < (*it).Begin) {
			if (end_offset >= end_patch_offset) {
				// all bytes of this patch are part of substring
				r->replace((*it).Begin-offset, (*it).Length, *((*it).Data), (*it).DataBegin, (*it).Length);
				copied += (*it).Length;
			} else {
				// substring ends in this patch
				patch_position sublen = end_offset-(*it).Begin;
				r->replace((*it).Begin-offset, sublen, *((*it).Data), (*it).DataBegin, sublen);
				copied += sublen;
			}
		}
	}

	if (copied != size) {
		throw NoDataHere();
	}

	return (r);
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
