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
#include "DPPropertyView.h"

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
		
		
		RN::UI::Menu *polygonModeMenu = new RN::UI::Menu();
		polygonModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Points"), RN::Number::WithInt32(static_cast<int32>(RN::Material::PolygonMode::Points))));
		polygonModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Lines"), RN::Number::WithInt32(static_cast<int32>(RN::Material::PolygonMode::Lines))));
		polygonModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Fill"), RN::Number::WithInt32(static_cast<int32>(RN::Material::PolygonMode::Fill))));
		
		InsertEnumWithTitle(RNCSTR("Polygon mode"), [material](int32 value) {
			material->SetPolygonMode(static_cast<RN::Material::PolygonMode>(value));
		}, static_cast<int32>(material->GetPolygonMode()), polygonModeMenu->Autorelease());
		
		
		InsertColorWithTitle(RNCSTR("Diffuse"), DPBindSetter(SetDiffuseColor), material->GetDiffuseColor());
		InsertColorWithTitle(RNCSTR("Ambient"), DPBindSetter(SetAmbientColor), material->GetAmbientColor());
		InsertColorWithTitle(RNCSTR("Specular"), DPBindSetter(SetSpecularColor), material->GetSpecularColor());
		InsertColorWithTitle(RNCSTR("Emissive"), DPBindSetter(SetEmissiveColor), material->GetEmissiveColor());
		
		InsertBooleanWithTitle(RNCSTR("Polygon offset"), DPBindSetter(SetPolygonOffset), material->GetPolygonOffset());
		InsertFloatWithTitle(RNCSTR("PO units"), DPBindSetter(SetPolygonOffsetUnits), material->GetPolygonOffsetUnits());
		InsertFloatWithTitle(RNCSTR("PO factor"), DPBindSetter(SetPolygonOffsetFactor), material->GetPolygonOffsetFactor());
		
		InsertBooleanWithTitle(RNCSTR("Depth test"), DPBindSetter(SetDepthTest), material->GetDepthTest());
		InsertBooleanWithTitle(RNCSTR("Depth write"), DPBindSetter(SetDepthWrite), material->GetDepthWrite());
		
		
		RN::UI::Menu *depthTestModeMenu = new RN::UI::Menu();
		depthTestModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Never"), RN::Number::WithInt32(static_cast<int32>(RN::Material::DepthMode::Never))));
		depthTestModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Always"), RN::Number::WithInt32(static_cast<int32>(RN::Material::DepthMode::Always))));
		depthTestModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Less"), RN::Number::WithInt32(static_cast<int32>(RN::Material::DepthMode::Less))));
		depthTestModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Less or Equal"), RN::Number::WithInt32(static_cast<int32>(RN::Material::DepthMode::LessOrEqual))));
		depthTestModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Equal"), RN::Number::WithInt32(static_cast<int32>(RN::Material::DepthMode::Equal))));
		depthTestModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Not Equal"), RN::Number::WithInt32(static_cast<int32>(RN::Material::DepthMode::NotEqual))));
		depthTestModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Greater or Equal"), RN::Number::WithInt32(static_cast<int32>(RN::Material::DepthMode::GreaterOrEqual))));
		depthTestModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Greater"), RN::Number::WithInt32(static_cast<int32>(RN::Material::DepthMode::Greater))));
		
		InsertEnumWithTitle(RNCSTR("Depth test mode"), [material](int32 value) {
			material->SetDepthTestMode(static_cast<RN::Material::DepthMode>(value));
		}, static_cast<int32>(material->GetDepthTestMode()), depthTestModeMenu->Autorelease());
		
		
		InsertBooleanWithTitle(RNCSTR("Double sided"), [material](bool value) {
			
			RN::Material::CullMode mode = value ? RN::Material::CullMode::None : RN::Material::CullMode::BackFace;
			material->SetCullMode(mode);
			
		}, (material->GetCullMode() == RN::Material::CullMode::None));
		
		InsertBooleanWithTitle(RNCSTR("Discard"), DPBindSetter(SetDiscard), material->GetDiscard());
		InsertFloatWithTitle(RNCSTR("Discard threshold"), DPBindSetter(SetDiscardThreshold), material->GetDiscardThreshold());
		
		InsertBooleanWithTitle(RNCSTR("Blending"), DPBindSetter(SetBlending), material->GetBlending());
		
		
		RN::UI::Menu *blendModeMenu = new RN::UI::Menu();
		blendModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Zero"), RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::Zero))));
		blendModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("One"), RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::One))));
		blendModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Source Color"), RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::SourceColor))));
		blendModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("One minus Source Color"), RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::OneMinusSourceColor))));
		blendModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Source Alpha"), RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::SourceAlpha))));
		blendModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("One minus Source Alpha"), RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::OneMinusSourceAlpha))));
		blendModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Destination Color"), RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::DestinationColor))));
		blendModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("One minus Destination Color"), RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::OneMinusDestinationColor))));
		blendModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Destination Alpha"), RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::DestinationAlpha))));
		blendModeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("One minus Destination Alpha"), RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::OneMinusDestinationAlpha))));
		
		blendModeMenu->Autorelease();
		
		InsertEnumWithTitle(RNCSTR("Blend source mode"), [material](int32 value) {
			material->SetBlendMode(static_cast<RN::Material::BlendMode>(value), material->GetBlendDestination());
		}, static_cast<int32>(material->GetBlendSource()), blendModeMenu);
		
		InsertEnumWithTitle(RNCSTR("Blend destination mode"), [material](int32 value) {
			material->SetBlendMode(material->GetBlendSource(), static_cast<RN::Material::BlendMode>(value));
		}, static_cast<int32>(material->GetBlendDestination()), blendModeMenu);
		
		
		RN::UI::Menu *blendEquationMenu = new RN::UI::Menu();
		blendEquationMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Add"), RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendEquation::Add))));
		blendEquationMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Substract"), RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendEquation::Subtract))));
		blendEquationMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Reverse Substract"), RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendEquation::ReverseSubtract))));
		blendEquationMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Min"), RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendEquation::Min))));
		blendEquationMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Max"), RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendEquation::Max))));
		
		InsertEnumWithTitle(RNCSTR("Blend equation"), [material](int32 value) {
			material->SetBlendEquation(static_cast<RN::Material::BlendEquation>(value));
		}, static_cast<int32>(material->GetBlendEquation()), blendEquationMenu->Autorelease());
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
		RN::UI::TextField *valueField = RN::UI::TextField::WithType(RN::UI::TextField::Type::Bezel);
		valueField->SetFormatter((new RN::NumberFormatter())->Autorelease());
		valueField->SetValue(RN::Number::WithFloat(value));
		valueField->SetFrame(RN::Rect(0.0f, 0.0f, 0.0f, 20.0f));
		valueField->AddListener(RN::UI::Control::EventType::ValueChanged, [=](RN::UI::Control *control, RN::UI::Control::EventType event) {
			RN::UI::TextField *field = control->Downcast<RN::UI::TextField>();
			setter(field->GetValue()->Downcast<RN::Number>()->GetFloatValue());
		}, nullptr);
		
		InsertViewWithTitle(title, valueField);
	}
	
	void MaterialView::InsertColorWithTitle(RN::String *title, std::function<void (const RN::Color &)> &&setter, const RN::Color &color)
	{
		RN::UI::ColorView *colorView = new RN::UI::ColorView();
		colorView->SetFrame(RN::Rect(0.0f, 0.0f, 140.0f, 15.0f));
		colorView->AddListener(RN::UI::Control::EventType::ValueChanged, [=](RN::UI::Control *control, RN::UI::Control::EventType event) {
			
			RN::UI::ColorView *colorView = control->Downcast<RN::UI::ColorView>();
			const RN::Color &color = colorView->GetColor();
			
			setter(color);
			
		}, nullptr);
		colorView->SetColor(color);
		
		InsertViewWithTitle(title, colorView->Autorelease());
	}
	
	
	void MaterialView::InsertEnumWithTitle(RN::String *title, std::function<void (int32)> &&setter, int32 value, RN::UI::Menu *menu)
	{
		RN::UI::PopUpButton *popUpButton = new RN::UI::PopUpButton();
		popUpButton->SetFrame(RN::Rect(0.0f, 0.0f, 0.0f, 20.0f));
		popUpButton->SetMenu(menu);
		popUpButton->SetTitleColorForState(ColorScheme::GetColor(ColorScheme::Type::FileTree_Text), RN::UI::Control::State::Normal);
		
		menu->GetItems()->Enumerate<RN::UI::MenuItem>([popUpButton, value](RN::UI::MenuItem *item, size_t index, bool &stop){
			if(item->GetAssociatedObject(kDPEnumItemAssociatedKey)->Downcast<RN::Number>()->GetInt32Value() == value)
			{
				popUpButton->SetSelection(index);
				stop = true;
			}
		});
		
		popUpButton->AddListener(RN::UI::Control::EventType::ValueChanged, [=](RN::UI::Control *control, RN::UI::Control::EventType event) {
			RN::UI::PopUpButton *popUp = control->Downcast<RN::UI::PopUpButton>();
			setter(popUp->GetSelectedItem()->GetAssociatedObject(kDPEnumItemAssociatedKey)->Downcast<RN::Number>()->GetInt32Value());
		}, nullptr);
		
		InsertViewWithTitle(title, popUpButton->Autorelease());
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
		float width = GetBounds().width;
		
		_textureViews->Enumerate<RN::Array>([&](RN::Array *views, size_t index, bool &stop) {
			
			RN::UI::View *title   = views->GetObjectAtIndex<RN::UI::View>(0);
			RN::UI::View *content = views->GetObjectAtIndex<RN::UI::View>(1);
			
			title->SetFrame(RN::Rect(5.0f, yoffset, kDPTitleLabelOffset, kDPTextureSize));
			content->SetFrame(RN::Rect(kDPTitleLabelOffset + 10.0f, yoffset, kDPTextureSize, kDPTextureSize));
			
			yoffset += kDPTextureSize + 2.0f;
			
		});
		
		_views->Enumerate<RN::Array>([&](RN::Array *views, size_t index, bool &stop) {
			
			if(views->GetCount() <= 2)
			{
				RN::UI::View *title   = views->GetObjectAtIndex<RN::UI::View>(0);
				RN::UI::View *content = views->GetObjectAtIndex<RN::UI::View>(1);
				
				RN::Rect titleFrame = RN::Rect(5.0f, yoffset, kDPTitleLabelOffset, 20.0f);
				RN::Rect contentFrame = RN::Rect(kDPTitleLabelOffset + 10.0f, yoffset, width - kDPTitleLabelOffset - 25.0f, content->GetSizeThatFits().y);
				
				title->SetFrame(titleFrame);
				content->SetFrame(contentFrame);
				
				yoffset += std::max(titleFrame.height, contentFrame.height) + 2.0f;
			}
			else
			{
				RN::UI::View *title = views->GetObjectAtIndex<RN::UI::View>(0);
				RN::Rect titleFrame = RN::Rect(5.0f, yoffset, kDPTitleLabelOffset, 20.0f);
				title->SetFrame(titleFrame);
				
				yoffset += titleFrame.height;
				float xoffset = 0.0f;
				float fieldWidth = (width - (views->GetCount() - 1) / 2.0f * 15.0f) / ((views->GetCount() - 1) / 2.0f);
				RN::Rect contentFrame;
				
				for(size_t i = 1; i < views->GetCount(); i++)
				{
					if(i%2 == 0)
					{
						RN::UI::View *content = views->GetObjectAtIndex<RN::UI::View>(i);
						contentFrame = RN::Rect(xoffset, yoffset, fieldWidth, content->GetSizeThatFits().y);
						content->SetFrame(contentFrame);
						
						xoffset += fieldWidth + 2.0f;
					}
					else
					{
						RN::UI::View *content = views->GetObjectAtIndex<RN::UI::View>(i);
						contentFrame = RN::Rect(xoffset, yoffset, content->GetSizeThatFits().x, content->GetSizeThatFits().y);
						content->SetFrame(contentFrame);
						
						xoffset += contentFrame.width + 2.0f;
					}
				}
				
				yoffset += contentFrame.height + 2.0f;
			}
		});
	}
	
	RN::Vector2 MaterialView::GetSizeThatFits()
	{
		LayoutIfNeeded();
		
		float yoffset = _textureViews->GetCount() * (kDPTextureSize + 2.0f);
		float width = GetBounds().width;
		
		_views->Enumerate<RN::Array>([&](RN::Array *views, size_t index, bool &stop) {
			
			if(views->GetCount() <= 2)
			{
				RN::UI::View *content = views->GetObjectAtIndex<RN::UI::View>(1);
				yoffset += std::max(20.0f, content->GetSizeThatFits().y) + 2.0f;
			}
			else
			{
				RN::UI::View *content = views->GetObjectAtIndex<RN::UI::View>(0);
				yoffset += std::max(20.0f, content->GetSizeThatFits().y) + 2.0f;
				
				content = views->GetObjectAtIndex<RN::UI::View>(2);
				yoffset += content->GetSizeThatFits().y + 2.0f;
			}
			
		});
		
		return RN::Vector2(width, yoffset);
	}
	
	
	MaterialWidget::MaterialWidget(RN::Material *material) :
		RN::UI::Widget(RN::UI::Widget::Style::Titled | RN::UI::Widget::Style::Closable, RN::Rect(0.0f, 0.0f, 300.0f, 580.0f))
	{
		SetTitle(RNCSTR("Material editor"));
		
		_materialView = new MaterialView(material);
		_materialView->SetFrame(RN::Rect(0.0f, 0.0f, 300.0f, 0.0f));
		_materialView->SizeToFit();
		
		RN::UI::ScrollView *scrollView = new RN::UI::ScrollView();
		scrollView->SetFrame(RN::Rect(0.0f, 0.0f, 300.0f, 580.0f).Inset(5.0f, 5.0f));
		scrollView->SetContentSize(RN::Vector2(200.0f, _materialView->GetSizeThatFits().y));
		scrollView->AddSubview(_materialView->Autorelease());
		scrollView->SetDelegate(this);
		ScrollViewDidChangeScrollerInset(scrollView, scrollView->GetScrollerInsets());
		
		GetContentView()->AddSubview(scrollView->Autorelease());
		Center();
	}
	
	void MaterialWidget::ScrollViewDidChangeScrollerInset(RN::UI::ScrollView *scrollView, const RN::UI::EdgeInsets &insets)
	{
		_materialView->SetFrame(RN::Rect(0.0f, 0.0f, 300.0f - insets.right, 0.0f));
		_materialView->SizeToFit();
	}
}
