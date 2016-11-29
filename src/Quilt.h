
#ifndef SRC_QUILT_H_
#define SRC_QUILT_H_

#include <string>
#include <vector>
#include <exception>
#include <stdexcept>
#include <memory>

#include "Ternary.h"

#include <iostream>

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
public:
	std::shared_ptr<const std::string> Content;
	PatchContent(std::shared_ptr<const std::string> content);
};

//lint -esym(1712,Patch)
class Patch {
public:
	const patch_position Begin;
	const patch_position Length;
	std::shared_ptr<PatchContent> Data;
	const patch_position DataBegin;
	Patch(
			const patch_position begin,
			const patch_position length,
			std::shared_ptr<PatchContent> data,
			const patch_position data_begin
	)
	: Begin(begin)
	, Length(length)
	, Data(data)
	, DataBegin(data_begin)
	{
	}

	inline std::shared_ptr<Patch> Copy() const
	{
		return (std::make_shared<Patch>(Begin, Length, Data, DataBegin));
	}
};

typedef std::vector<std::shared_ptr<Patch> > quilt;

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
	std::shared_ptr<Patch> CachedPatch;
	patch_position CachedPatchBegin;
	patch_position CachedPatchEnd;

	Quilt();
	Quilt(const patch_position length, const patch_position coveredSize);

	// construct from string
	Quilt(std::shared_ptr<const std::string> data);
	Quilt(std::shared_ptr<const std::string> data, const patch_position length);

	// cut origin
	Quilt(std::shared_ptr<Quilt> origin, const patch_position offset, const patch_position length);
	Quilt(std::shared_ptr<Quilt> origin, const patch_position offset);

	// sew
	Quilt(const patch_position length);
	void Sew(std::shared_ptr<Quilt> origin, const patch_position offset, const bool resize);
	void SewWithHole(std::shared_ptr<Quilt> origin, const patch_position offset, const patch_position length);

	std::shared_ptr<Patch> GetPatch(patch_position offset)
	{
		if (offset >= CachedPatchBegin && offset < CachedPatchEnd)
			return (CachedPatch);

		for (quilt::iterator it = Data.begin(); it != Data.end(); ++it) {
			if ((*it)->Begin <= offset && (*it)->Begin + (*it)->Length > offset) {
				CachedPatch = (*it);
				CachedPatchBegin = (*it)->Begin;
				CachedPatchEnd = CachedPatchBegin + (*it)->Length;

				return (*it);
			}
		}

		return (std::shared_ptr<Patch>(nullptr));
	}

	inline unsigned char GetCharOrFail(patch_position offset)
	{
		const std::shared_ptr<Patch> p = GetPatch(offset);
		if (p) {
			return ((unsigned char)p->Data->Content->at((offset-p->Begin)+p->DataBegin));
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

	std::string *GetSubStringOrFail(const patch_position offset, const patch_position size) const;
	std::string GetMaxSubString(const patch_position offset, const patch_position size);
	void CopyBytesOrFail(char *buffer, const patch_position offset, const patch_position size) const;
	const ternary::Ternary &CompareChar(patch_position offset, const unsigned char with);
	const ternary::Ternary &CompareShortLE(patch_position offset, const unsigned short with);
	const ternary::Ternary &CompareShortBE(patch_position offset, const unsigned short with);
	const ternary::Ternary &CompareSubString(patch_position offset, const std::string &with) const;
private:
	inline void AddPatch(std::shared_ptr<Patch> p)
	{
		Data.push_back(p);
	}

	void AddNewPatch(
			const patch_position begin,
			const patch_position length,
			std::shared_ptr<PatchContent> data,
			const patch_position data_begin);
	void Cut(std::shared_ptr<Quilt> origin, const patch_position offset, const patch_position length);
};

#endif /* SRC_QUILT_H_ */
