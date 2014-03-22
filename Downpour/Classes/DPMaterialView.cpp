//
//  DPMaterialView.cpp
//  Downpour
//
//  Copyright 2014 by Überpixel. All rights reserved.
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
//  documentation files (the "Software"), to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
//  and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
//  PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
//  FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "DPMaterialView.h"
#include "DPColorScheme.h"

#define kDPTitleLabelOffset (80.0f)
#define kDPTextureSize      (70.0f)

#define DPBindSetter(name) (std::bind(&RN::Material::name, _material, std::placeholders::_1))

namespace DP
{
	RNDefineMeta(MaterialView, RN::UI::View)
	RNDefineMeta(MaterialWidget, RN::UI::Widget)
	
	MaterialView::MaterialView(RN::Material *material) :
		_material(material->Retain()),
		_textureViews(new RN::Array()),
		_views(new RN::Array())
	{
		const RN::Array *textures = _material->GetTextures();
		
		textures->Enumerate<RN::Texture>([&](RN::Texture *texture, size_t index, bool &stop) {
			InsertTexture(texture);
		});
		
		InsertBooleanWithTitle(RNCSTR("Lighting"), DPBindSetter(SetLighting), material->GetLighting());
		InsertBooleanWithTitle(RNCSTR("Polygon offset"), DPBindSetter(SetPolygonOffset), material->GetPolygonOffset());
		InsertBooleanWithTitle(RNCSTR("Double sided"), [material](bool value) {
			
			RN::Material::CullMode mode = value ? RN::Material::CullMode::None : RN::Material::CullMode::BackFace;
			material->SetCullMode(mode);
			
		}, (material->GetCullMode() == RN::Material::CullMode::None));
		
		InsertFloatWithTitle(RNCSTR("PO units"), DPBindSetter(SetPolygonOffsetUnits), material->GetPolygonOffsetUnits());
		InsertFloatWithTitle(RNCSTR("PO factor"), DPBindSetter(SetPolygonOffsetFactor), material->GetPolygonOffsetFactor());
		InsertBooleanWithTitle(RNCSTR("Depth test"), DPBindSetter(SetDepthTest), material->GetDepthTest());
		InsertBooleanWithTitle(RNCSTR("Depth write"), DPBindSetter(SetDepthWrite), material->GetDepthWrite());
		InsertBooleanWithTitle(RNCSTR("Discard"), DPBindSetter(SetDiscard), material->GetDiscard());
		InsertFloatWithTitle(RNCSTR("Discard threshold"), DPBindSetter(SetDiscardThreshold), material->GetDiscardThreshold());
	}
	
	MaterialView::~MaterialView()
	{
		_views->Release();
		_textureViews->Release();
		
		_material->Release();
	}
	
	
	
	void MaterialView::InsertTexture(RN::Texture *texture)
	{
		RN::UI::ImageView *textureView = new RN::UI::ImageView(RN::UI::Image::WithTexture(texture));
		
		RN::UI::Label *label = new RN::UI::Label();
		label->SetAlignment(RN::UI::TextAlignment::Right);
		label->SetNumberOfLines(0);
		label->SetLineBreak(RN::UI::LineBreakMode::WordWrapping);
		label->SetText(RNSTR(texture->::RN::Asset::GetName().c_str()));
		label->SetTextColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_Text));
		
		_textureViews->AddObject(RN::Array::WithObjects(label->Autorelease(), textureView->Autorelease(), nullptr));
		
		AddSubview(label);
		AddSubview(textureView);
	}
	
	
	void MaterialView::InsertBooleanWithTitle(RN::String *title, std::function<void (bool)> &&setter, bool state)
	{
		RN::UI::Button *button = RN::UI::Button::WithType(RN::UI::Button::Type::CheckBox);
		button->SizeToFit();
		button->SetSelected(state);
		button->AddListener(RN::UI::Control::EventType::MouseUpInside, [=](RN::UI::Control *control, RN::UI::Control::EventType event) {
			setter(control->IsSelected());
		}, nullptr);
		
		InsertViewWithTitle(title, button);
	}
	
	void MaterialView::InsertFloatWithTitle(RN::String *title, std::function<void (float)> &&setter, float value)
	{
		RN::UI::TextField *valueField = RN::UI::TextField::WithType(RN::UI::TextField::Type::Bezel)->Retain();
		valueField->SetFormatter((new RN::NumberFormatter())->Autorelease());
		valueField->SetValue(RN::Number::WithFloat(value));
		valueField->SetFrame(RN::Rect(0.0f, 0.0f, 0.0f, 20.0f));
		
		InsertViewWithTitle(title, valueField);
	}
	
	
	void MaterialView::InsertViewWithTitle(RN::String *title, RN::UI::View *view)
	{
		RN::UI::Label *label = new RN::UI::Label();
		label->SetAlignment(RN::UI::TextAlignment::Right);
		label->SetText(title);
		label->SetTextColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_Text));
		
		_views->AddObject(RN::Array::WithObjects(label->Autorelease(), view, nullptr));
		
		AddSubview(label);
		AddSubview(view);
	}
	
	void MaterialView::LayoutSubviews()
	{
		RN::UI::View::LayoutSubviews();
		
		float yoffset = 0.0f;
		float width   = GetBounds().width;
		
		_textureViews->Enumerate<RN::Array>([&](RN::Array *views, size_t index, bool &stop) {
			
			RN::UI::View *title   = views->GetObjectAtIndex<RN::UI::View>(0);
			RN::UI::View *content = views->GetObjectAtIndex<RN::UI::View>(1);
			
			title->SetFrame(RN::Rect(5.0f, yoffset, kDPTitleLabelOffset, kDPTextureSize));
			content->SetFrame(RN::Rect(kDPTitleLabelOffset + 10.0f, yoffset, kDPTextureSize, kDPTextureSize));
			
			yoffset += kDPTextureSize + 2.0f;
			
		});
		
		_views->Enumerate<RN::Array>([&](RN::Array *views, size_t index, bool &stop) {
			
			RN::UI::View *title   = views->GetObjectAtIndex<RN::UI::View>(0);
			RN::UI::View *content = views->GetObjectAtIndex<RN::UI::View>(1);
			
			RN::Rect titleFrame = RN::Rect(5.0f, yoffset, kDPTitleLabelOffset, 20.0f);
			RN::Rect contentFrame = RN::Rect(kDPTitleLabelOffset + 10.0f, yoffset, width - (kDPTitleLabelOffset - 25.0f), content->GetSizeThatFits().y);
			
			title->SetFrame(titleFrame);
			content->SetFrame(contentFrame);
			
			yoffset += std::max(titleFrame.height, contentFrame.height) + 2.0f;
			
		});
	}
	
	RN::Vector2 MaterialView::GetSizeThatFits()
	{
		float yoffset = _textureViews->GetCount() * (kDPTextureSize + 2.0f);
		float width   = GetBounds().width;
		
		_views->Enumerate<RN::Array>([&](RN::Array *views, size_t index, bool &stop) {
			
			RN::UI::View *content = views->GetObjectAtIndex<RN::UI::View>(1);
			yoffset += std::max(20.0f, content->GetSizeThatFits().y) + 2.0f;
			
		});
		
		return RN::Vector2(width, yoffset);
	}
	
	
	MaterialWidget::MaterialWidget(RN::Material *material) :
		RN::UI::Widget(RN::UI::Widget::StyleTitled | RN::UI::Widget::StyleClosable, RN::Rect(0.0f, 0.0f, 300.0f, 580.0f))
	{
		SetTitle(RNCSTR("Material editor"));
		
		MaterialView *view = new MaterialView(material);
		view->SetFrame(RN::Rect(0.0f, 0.0f, 300.0f, 0.0f));
		view->SizeToFit();
		
		RN::UI::ScrollView *scrollView = new RN::UI::ScrollView();
		scrollView->SetFrame(RN::Rect(0.0f, 0.0f, 300.0f, 580.0f).Inset(5.0f, 5.0f));
		scrollView->SetContentSize(RN::Vector2(200.0f, view->GetSizeThatFits().y));
		scrollView->AddSubview(view->Autorelease());
		
		GetContentView()->AddSubview(scrollView->Autorelease());
		Center();
	}
}
