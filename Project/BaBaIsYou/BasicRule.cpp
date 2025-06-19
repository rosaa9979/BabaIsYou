#pragma once
#include "BasicRule.h"
#include "Stage.h"
#include "GameManager.h"

using namespace std;

BasicRule::BasicRule(Text* subject, Text* verb, Text* predicate, RuleType type)
	: subject(subject), verb(verb), predicate(predicate), type(type) {
}

wstring BasicRule::ToString() const
{
    std::string s = subject->GetId();
    std::string v = verb->GetId();
    std::string p = predicate->GetId();

    return std::wstring(s.begin(), s.end()) + L" " +
        std::wstring(v.begin(), v.end()) + L" " +
        std::wstring(p.begin(), p.end()) + L"\n";
}

RuleType BasicRule::GetRuleType() const
{
	return type;
}

void BasicRule::Apply(Stage& stage)
{
    MaterialType subjectType = subject->ToMaterialType();
    ElementStatus effect = predicate->GetEffect();

    for (Element* ele : stage.GetElements())
    {
        if (!ele->IsMaterial()) continue;

        Material* mat = static_cast<Material*>(ele);

        if (mat->GetMaterialType() != subjectType) continue;

        if (type == RuleType::Property) {
            mat->SetStatus(effect);
        }
        else if (type == RuleType::Identity) {
            MaterialType newType = predicate->ToMaterialType();
            stage.ReplaceElementWithType(ele, newType);
        }
    }
}