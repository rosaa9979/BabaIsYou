#include <algorithm>
#include "Text.h"
#include "Stage.h"
#include "Rule.h"
#include "BasicRule.h"

const std::unordered_map<std::string, MaterialType> Text::nameToType = {
	{ "none", MaterialType::None },
	{ "baba", MaterialType::BaBa },
	{ "flag", MaterialType::Flag },
	{ "wall", MaterialType::Wall },
	{ "rock", MaterialType::Rock}
};

using namespace std;

Text::Text(string _id, wstring _image_path, Color _color, TextType _type, ElementStatus _effect, ElementStatus _status)
	: Element(_id, _image_path, _color, _status)
{
	type = _type; // �⺻�� ����
	effect = _effect;
}

void Text::Update()
{
	Element::Update();
    status = { ElementStatus::Push };

    Element::UpdatePosition();
}

ElementStatus Text::GetEffect() const
{
	if (type == TextType::Property)
		return effect;
}

string Text::GetObjString() const
{
    // Noun Ÿ���� �ƴ� ���� object�� �ƴϹǷ� �� ���ڿ� ��ȯ
    if (GetTextType() != TextType::Noun) {
        return "";
    }

    const string& id = GetId(); // ��: "Text_Wall"
    const string prefix = "text_";
    std::string result;

    // "Text_"�� �����ϸ� ���ξ� ����
    if (id.rfind(prefix, 0) == 0) {
        result = id.substr(prefix.length()); // ��: "Wall"
    }
    else {
        result = id; // ���������� ���ξ� ������ �״�� ���
    }

    // �ҹ��� ��ȯ
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });

    return result; // ���: "wall"
}

MaterialType Text::ToMaterialType() const {
	return FromString(GetObjString());
}

MaterialType Text::FromString(const string& name) {
	auto it = nameToType.find(name);
	if (it != nameToType.end()) return it->second;
	return MaterialType::None;
}

void Text::Render(const RenderContext& ctx)
{
	Element::Render(ctx);
}

bool Text::IsActiveInStage(Stage* stage) const
{
    for (const auto& rulePtr : stage->GetRules()) {
        const Rule* rule = rulePtr.get();  // ✅ unique_ptr → raw pointer

        if (auto* basic = dynamic_cast<const BasicRule*>(rule)) {
            if (basic->GetSubject() == this || basic->GetVerb() == this || basic->GetPredicate() == this) {
                return true;
            }
        }
    }
    return false;
}
