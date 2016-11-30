// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef SRC_TERNARY_H_
#define SRC_TERNARY_H_

#include <exception>
#include <stdexcept>

namespace ternary {

class UnknownValue : public std::runtime_error {
public:
	UnknownValue()
		: std::runtime_error("UnknownValue")
	{

	}

	virtual const char* what() const throw()
	{
		return ("UnknownValue");
	}
};

enum values { IsTrue, IsFalse, IsUnknown };

//lint -esym(1712,Ternary)
//lint -e1739
//lint -e1753
//lint -e1714
class Ternary {
public:
	const enum values Value;
	Ternary(values v);
	Ternary(bool v);
	const bool operator==(const Ternary &second) const;
	const bool operator==(bool second) const;
	const Ternary &operator&&(const Ternary &second) const;
	const Ternary &operator&&(bool second) const;
	const Ternary &operator||(const Ternary &second) const;
	const Ternary &operator||(bool second) const;
	const Ternary &operator!() const;
	const bool ProbablyTrue() const;
	const bool ProbablyFalse() const;
	const bool BoolOrFail() const;
};

const Ternary True = Ternary(IsTrue);
const Ternary False = Ternary(IsFalse);
const Ternary Unknown = Ternary(IsUnknown);

} /* namespace ternary */

#endif /* SRC_TERNARY_H_ */
