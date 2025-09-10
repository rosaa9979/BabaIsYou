#include "Material.h"

Material::Material(string _id, wstring _image_path, Color _color, MaterialType _type, ElementStatus _status) : Element(_id, _image_path, _color, _status)
{
	type = _type;
}

void Material::Update()
{
	Element::Update();

	if (id == "Border")
		return;

	Element::UpdatePosition();
}

void Material::Render(const RenderContext& ctx)
{
	Element::Render(ctx);
}
