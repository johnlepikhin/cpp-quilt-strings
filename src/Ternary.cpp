
#include "Ternary.h"

namespace ternary {

Ternary::Ternary(enum values v)
	: Value(v)
{
}

Ternary::Ternary(bool v)
	: Value((v) ? IsTrue : IsFalse)
{
}

const bool Ternary::operator==(const Ternary &second) const {
	return (Value == second.Value);
}

const bool Ternary::operator==(bool second) const {
	if (Value == IsTrue) {
		if (second) {
			return (true);
		} else {
			return (false);
		}
	} else if (Value == IsFalse) {
		if (second) {
			return (false);
		} else {
			return (true);
		}
	} else {
		return (false);
	}
}

const Ternary &Ternary::operator&&(const Ternary &second) const {
	if (Value == IsTrue) {
		return (second);
	} else if (Value == IsFalse) {
		return (False);
	} else {
		if (second.Value == IsFalse) {
			return (False);
		} else {
			return (Unknown);
		}
	}
}

const Ternary &Ternary::operator&&(bool second) const {
	if (Value == IsTrue) {
		if (second) {
			return (True);
		} else {
			return (False);
		}
	} else if (Value == IsFalse) {
		return (False);
	} else {
		if (!second) {
			return (False);
		} else {
			return (Unknown);
		}
	}
}

const Ternary &Ternary::operator||(const Ternary &second) const {
	if (Value == IsTrue) {
		return (True);
	} else if (Value == IsFalse) {
		return (second);
	} else {
		if (second.Value == IsTrue) {
			return (True);
		} else {
			return (Unknown);
		}
	}
}

const Ternary &Ternary::operator||(bool second) const {
	if (Value == IsTrue) {
		return (True);
	} else if (Value == IsFalse) {
		if (second) {
			return (True);
		} else {
			return (False);
		}
	} else {
		if (second) {
			return (True);
		} else {
			return (Unknown);
		}
	}
}

const Ternary &Ternary::operator!() const {
	if (Value == IsTrue) {
		return (False);
	} else if (Value == IsFalse) {
		return (True);
	} else {
		return (Unknown);
	}
}

const bool Ternary::ProbablyTrue() const {
	return ((Value == IsFalse) ? false : true);
}

const bool Ternary::ProbablyFalse() const {
	return ((Value == IsTrue) ? true : false);
}

const bool Ternary::BoolOrFail() const {
	if (Value == IsTrue) {
		return (true);
	} else if (Value == IsFalse) {
		return (false);
	} else {
		throw UnknownValue();
	}
}

} /* namespace ternary */
