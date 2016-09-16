
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

class PatchContent {
private:
	int RefCounter;
	const std::string *Content;
public:
	PatchContent(const std::string *content);
	~PatchContent();
	inline const std::string *Get()
	{
		return (Content);
	};
	void StartUse();
	int StopUse();
};

//lint -esym(1712,Patch)
class Patch {
public:
	const patch_position Begin;
	const patch_position Length;
	PatchContent *Data;
	const patch_position DataBegin;
	Patch(
			const patch_position begin,
			const patch_position length,
			PatchContent *data,
			const patch_position data_begin
	)
	: Begin(begin)
	, Length(length)
	, Data(data)
	, DataBegin(data_begin)
	{
		Data->StartUse();
	}

	~Patch()
	{
		if (!Data->StopUse())
		{
			delete Data;
		}
	}

	Patch *Copy() const
	{
		return (new Patch(Begin, Length, Data, DataBegin));
	}
};

typedef std::vector<const Patch *> quilt;

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
//lint -sem(Quilt::AddPatch, 1p, custodial(1))
class Quilt {
public:
	quilt Data;
	patch_position Length;
	patch_position CoveredSize;

	Quilt();
	Quilt(const patch_position length, const patch_position coveredSize);

	~Quilt();

	inline const Patch *GetPatch(patch_position offset) const
	{
		for (quilt::const_iterator it = Data.begin(); it != Data.end(); ++it) {
			if ((*it)->Begin <= offset && (*it)->Begin + (*it)->Length > offset) {
				return (*it);
			}
		}

		return (NULL);
	}

	inline unsigned char GetCharOrFail(patch_position offset) const
	{
		const Patch *p = GetPatch(offset);
		if (p) {
			return ((unsigned char)p->Data->Get()->at((offset-p->Begin)+p->DataBegin));
		} else {
			throw NoDataHere();
		}
	}

	inline unsigned short GetShortBEOrFail(patch_position offset) const
	{
		unsigned short r = GetCharOrFail(offset) << 8;
		r += GetCharOrFail(offset+1);

		return (r);
	}

	inline unsigned short GetShortLEOrFail(patch_position offset) const
	{
		unsigned short r = GetCharOrFail(offset+1) << 8;
		r += GetCharOrFail(offset);

		return (r);
	}

	std::string *GetSubStringOrFail(const patch_position offset, const patch_position size) const;
	void CopyBytesOrFail(char *buffer, const patch_position offset, const patch_position size) const;

	const ternary::Ternary &CompareChar(patch_position offset, const unsigned char with) const;
	const ternary::Ternary &CompareShortLE(patch_position offset, const unsigned short with) const;
	const ternary::Ternary &CompareShortBE(patch_position offset, const unsigned short with) const;
	const ternary::Ternary &CompareSubString(patch_position offset, const std::string &with) const;
protected:
	void AddPatch(Patch *p);
	void AddNewPatch(
			const patch_position begin,
			const patch_position length,
			PatchContent *data,
			const patch_position data_begin);
};

//lint -esym(1712,QuiltSnippet)
//lint -esym(1509,QuiltSnippet)
class QuiltSnippet : public Quilt {
public:
	QuiltSnippet(const std::string *data);
	QuiltSnippet(const std::string *data, const patch_position length);
};

//lint -esym(1712,QuiltCut)
//lint -esym(1509,QuiltCut)
class QuiltCut : public Quilt {
public:
	QuiltCut(const Quilt *origin, const patch_position offset, const patch_position length);
	QuiltCut(const Quilt *origin, const patch_position offset);
private:
	void Cut(const Quilt *origin, const patch_position offset, const patch_position length);
};

//lint -esym(1712,QuiltSew)
//lint -esym(1509,QuiltSew)
class QuiltSew : public Quilt {
public:
	QuiltSew(const patch_position length);
	void Sew(const Quilt *origin, const patch_position offset);
};

#endif /* SRC_QUILT_H_ */
