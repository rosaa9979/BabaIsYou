#pragma once
#include <unordered_map>
#include "Element.h"
#include "Material.h"

using namespace std;

enum class TextType
{
	Noun,
	Verb,
	Property
};

class Text : public Element
{
private:
	TextType type;
	ElementStatus effect;
	static const unordered_map<string, MaterialType> nameToType;

public:
	Text(string _id, wstring _image_path, Color _color, TextType _type, ElementStatus _effect = ElementStatus::None, ElementStatus _status = ElementStatus::Push);
	virtual void Update() override;
	virtual void Reset() override { Element::Reset(); status = { ElementStatus::Push }; }
	virtual Element* Clone() const override { return new Text(*this); }
	virtual bool IsText() const override { return true; }
	TextType GetTextType() const { return type; }
	ElementStatus GetEffect() const;
	string GetObjString() const;
	virtual bool IsAlwaysPushable() const override { return true; } 
	MaterialType ToMaterialType() const;
	static MaterialType FromString(const string& name);
	void Render(const RenderContext& ctx);
	bool IsActiveInStage(Stage* stage) const;

};