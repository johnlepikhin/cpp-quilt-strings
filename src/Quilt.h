
#ifndef SRC_QUILT_H_
#define SRC_QUILT_H_

#include <string>
#include <vector>
#include <exception>
#include <stdexcept>

#include "Ternary.h"

class NoDataHere : public std::runtime_error {
public:
	NoDataHere()
		: std::runtime_error("NoDataHere")
	{

	}

	virtual const char* what() const throw()
	{
		return ("NoDataHere");
	}
};

typedef unsigned long int patch_position;

//lint -esym(1712,Patch)
class Patch {
public:
	patch_position Begin;
	patch_position Length;
	std::string *Data;
	patch_position DataBegin;
	Patch(
			patch_position begin,
			patch_position length,
			std::string *data,
			patch_position data_begin
	)
	: Begin(begin)
	, Length(length)
	, Data(data)
	, DataBegin(data_begin)
	{
	}

	Patch Copy()
	{
		return (Patch(Begin, Length, Data, DataBegin));
	}
};

typedef std::vector<Patch> quilt;

//lint -esym(1714,Quilt::GetCharOrFail)
//lint -esym(1714,Quilt::GetSubStringOrFail)
//lint -esym(1714,Quilt::GetShortBEOrFail)
//lint -esym(1714,Quilt::GetShortLEOrFail)
//lint -esym(1714,Quilt::CompareChar)
//lint -esym(1714,Quilt::CompareShortBE)
//lint -esym(1714,Quilt::CompareShortLE)
//lint -esym(1714,Quilt::CompareSubString)
//lint -esym(1790,Quilt)
//lint -esym(1512,Quilt)
class Quilt {
public:
	quilt Data;
	patch_position Length;
	patch_position CoveredSize;

	~Quilt() {};

	inline Patch *GetPatch(patch_position offset)
	{
		for (quilt::iterator it = Data.begin(); it != Data.end(); ++it) {
			if ((*it).Begin <= offset && (*it).Begin + (*it).Length > offset) {
				return (&(*it));
			}
		}

		return (NULL);
	}

	inline unsigned char GetCharOrFail(patch_position offset)
	{
		Patch *p = GetPatch(offset);
		if (p) {
			return ((unsigned char)p->Data->at((offset-p->Begin)+p->DataBegin));
		} else {
			throw NoDataHere();
		}
	}

	inline unsigned short GetShortBEOrFail(patch_position offset)
	{
		unsigned short r = GetCharOrFail(offset) << 8;
		r += GetCharOrFail(offset+1);

		return (r);
	}

	inline unsigned short GetShortLEOrFail(patch_position offset)
	{
		unsigned short r = GetCharOrFail(offset+1) << 8;
		r += GetCharOrFail(offset);

		return (r);
	}

	std::string *GetSubStringOrFail(patch_position offset, patch_position size);
	void CopyBytesOrFail(char *buffer, patch_position offset, patch_position size);

	const ternary::Ternary &CompareChar(patch_position offset, const unsigned char with);
	const ternary::Ternary &CompareShortLE(patch_position offset, const unsigned short with);
	const ternary::Ternary &CompareShortBE(patch_position offset, const unsigned short with);
	const ternary::Ternary &CompareSubString(patch_position offset, const std::string &with);
};

//lint -esym(1712,QuiltSnippet)
//lint -esym(1509,QuiltSnippet)
class QuiltSnippet : public Quilt {
public:
	QuiltSnippet(std::string *data);
};

//lint -esym(1712,QuiltCut)
//lint -esym(1509,QuiltCut)
class QuiltCut : public Quilt {
public:
	QuiltCut(Quilt *origin, patch_position offset, patch_position length);
	QuiltCut(Quilt *origin, patch_position offset);
private:
	void Cut(Quilt *origin, patch_position offset, patch_position length);
};

//lint -esym(1712,QuiltSew)
//lint -esym(1509,QuiltSew)
class QuiltSew : public Quilt {
public:
	QuiltSew(patch_position length);
	void Sew(Quilt *origin, patch_position offset);
};

#endif /* SRC_QUILT_H_ */
