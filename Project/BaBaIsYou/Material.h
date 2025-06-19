#pragma once
#include "Element.h"

enum class MaterialType
{
	None, 
	BaBa,
	Flag,
	Wall,
	Rock
};

class Material : public Element
{
private:
	MaterialType type;

public:
	Material(string _id, wstring _image_path, Color _color, MaterialType _type, ElementStatus _status = ElementStatus::None);
	virtual void Update() override;
	virtual void Reset() override { Element::Reset(); status = { ElementStatus::None }; }
	virtual Element* Clone() const override { return new Material(*this); }
	virtual bool IsMaterial() const override { return true; }
	MaterialType GetMaterialType() const { return type; }
	void Render(const RenderContext& ctx);
};