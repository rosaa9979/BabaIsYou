#pragma once
#include <string>
#include "Rule.h"
#include "Text.h"
#include "Material.h"

using namespace std;

class BasicRule : public Rule {
private:
    Text* subject;
    Text* verb;
    Text* predicate;
    RuleType type;

public:
    BasicRule(Text* subject, Text* verb, Text* predicate, RuleType type);
    wstring ToString() const override;
    RuleType GetRuleType() const;
    void Apply(Stage& stage) override;
	inline Text* GetSubject() const { return subject; }
	inline Text* GetVerb() const { return verb; }
	inline Text* GetPredicate() const { return predicate; }
};
#pragma once
