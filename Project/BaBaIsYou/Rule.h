#pragma once
#include "Text.h"

enum class RuleType
{
	Identity,
	Property
};

class Rule {
public:
    virtual std::wstring ToString() const = 0;
    virtual ~Rule() = default;
	virtual void Apply(Stage& stage) = 0;
};