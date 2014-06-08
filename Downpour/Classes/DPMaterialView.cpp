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

const char *kDPMenuItemAssociatedKey = "kDPMenuItemAssociatedKey";

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
		RN::UI::MenuItem *item = RN::UI::MenuItem::WithTitle(RNCSTR("Points"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::PolygonMode::Points)), RN::Object::MemoryPolicy::Retain);
		polygonModeMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Lines"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::PolygonMode::Lines)), RN::Object::MemoryPolicy::Retain);
		polygonModeMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Fill"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::PolygonMode::Fill)), RN::Object::MemoryPolicy::Retain);
		polygonModeMenu->AddItem(item);
		
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
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Never"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::DepthMode::Never)), RN::Object::MemoryPolicy::Retain);
		depthTestModeMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Always"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::DepthMode::Always)), RN::Object::MemoryPolicy::Retain);
		depthTestModeMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Less"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::DepthMode::Less)), RN::Object::MemoryPolicy::Retain);
		depthTestModeMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Less or Equal"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::DepthMode::LessOrEqual)), RN::Object::MemoryPolicy::Retain);
		depthTestModeMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Equal"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::DepthMode::Equal)), RN::Object::MemoryPolicy::Retain);
		depthTestModeMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Not Equal"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::DepthMode::NotEqual)), RN::Object::MemoryPolicy::Retain);
		depthTestModeMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Greater or Equal"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::DepthMode::GreaterOrEqual)), RN::Object::MemoryPolicy::Retain);
		depthTestModeMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Greater"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::DepthMode::Greater)), RN::Object::MemoryPolicy::Retain);
		depthTestModeMenu->AddItem(item);
		
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
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Zero"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::Zero)), RN::Object::MemoryPolicy::Retain);
		blendModeMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("One"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::One)), RN::Object::MemoryPolicy::Retain);
		blendModeMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Source Color"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::SourceColor)), RN::Object::MemoryPolicy::Retain);
		blendModeMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("One minus Source Color"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::OneMinusSourceColor)), RN::Object::MemoryPolicy::Retain);
		blendModeMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Source Alpha"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::SourceAlpha)), RN::Object::MemoryPolicy::Retain);
		blendModeMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("One minus Source Alpha"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::OneMinusSourceAlpha)), RN::Object::MemoryPolicy::Retain);
		blendModeMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Destination Color"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::DestinationColor)), RN::Object::MemoryPolicy::Retain);
		blendModeMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("One minus Destination Color"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::OneMinusDestinationColor)), RN::Object::MemoryPolicy::Retain);
		blendModeMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Destination Alpha"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::DestinationAlpha)), RN::Object::MemoryPolicy::Retain);
		blendModeMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("One minus Destination Alpha"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendMode::OneMinusDestinationAlpha)), RN::Object::MemoryPolicy::Retain);
		blendModeMenu->AddItem(item);
		blendModeMenu->Autorelease();
		
		InsertEnumWithTitle(RNCSTR("Blend source mode"), [material](int32 value) {
			material->SetBlendMode(static_cast<RN::Material::BlendMode>(value), material->GetBlendDestination());
		}, static_cast<int32>(material->GetBlendSource()), blendModeMenu);
		
		InsertEnumWithTitle(RNCSTR("Blend destination mode"), [material](int32 value) {
			material->SetBlendMode(material->GetBlendSource(), static_cast<RN::Material::BlendMode>(value));
		}, static_cast<int32>(material->GetBlendDestination()), blendModeMenu);

		
		RN::UI::Menu *blendEquationMenu = new RN::UI::Menu();
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Add"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendEquation::Add)), RN::Object::MemoryPolicy::Retain);
		blendEquationMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Substract"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendEquation::Subtract)), RN::Object::MemoryPolicy::Retain);
		blendEquationMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Reverse Substract"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendEquation::ReverseSubtract)), RN::Object::MemoryPolicy::Retain);
		blendEquationMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Min"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendEquation::Min)), RN::Object::MemoryPolicy::Retain);
		blendEquationMenu->AddItem(item);
		
		item = RN::UI::MenuItem::WithTitle(RNCSTR("Max"));
		item->SetAssociatedObject(kDPMenuItemAssociatedKey, RN::Number::WithInt32(static_cast<int32>(RN::Material::BlendEquation::Max)), RN::Object::MemoryPolicy::Retain);
		blendEquationMenu->AddItem(item);
		
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
		RN::UI::TextField *valueField = RN::UI::TextField::WithType(RN::UI::TextField::Type::Bezel)->Retain();
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
		RN::UI::Label *label = new RN::UI::Label();
		label->SetAlignment(RN::UI::TextAlignment::Left);
		label->SetText(title);
		label->SetTextColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_Text));
		
		RN::UI::Label *redLabel = new RN::UI::Label();
		redLabel->SetAlignment(RN::UI::TextAlignment::Right);
		redLabel->SetText(RNCSTR("R"));
		redLabel->SetTextColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_Text));
		
		RN::UI::Label *greenLabel = new RN::UI::Label();
		greenLabel->SetAlignment(RN::UI::TextAlignment::Right);
		greenLabel->SetText(RNCSTR("G"));
		greenLabel->SetTextColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_Text));
		
		RN::UI::Label *blueLabel = new RN::UI::Label();
		blueLabel->SetAlignment(RN::UI::TextAlignment::Right);
		blueLabel->SetText(RNCSTR("B"));
		blueLabel->SetTextColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_Text));
		
		RN::UI::Label *alphaLabel = new RN::UI::Label();
		alphaLabel->SetAlignment(RN::UI::TextAlignment::Right);
		alphaLabel->SetText(RNCSTR("A"));
		alphaLabel->SetTextColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_Text));
		
		
		RN::UI::TextField *redField = RN::UI::TextField::WithType(RN::UI::TextField::Type::Bezel)->Retain();
		redField->SetFormatter((new RN::NumberFormatter())->Autorelease());
		redField->SetValue(RN::Number::WithFloat(color.r));
		redField->SetFrame(RN::Rect(0.0f, 0.0f, 0.0f, 20.0f));
		
		RN::UI::TextField *greenField = RN::UI::TextField::WithType(RN::UI::TextField::Type::Bezel)->Retain();
		greenField->SetFormatter((new RN::NumberFormatter())->Autorelease());
		greenField->SetValue(RN::Number::WithFloat(color.g));
		greenField->SetFrame(RN::Rect(0.0f, 0.0f, 0.0f, 20.0f));
		
		RN::UI::TextField *blueField = RN::UI::TextField::WithType(RN::UI::TextField::Type::Bezel)->Retain();
		blueField->SetFormatter((new RN::NumberFormatter())->Autorelease());
		blueField->SetValue(RN::Number::WithFloat(color.b));
		blueField->SetFrame(RN::Rect(0.0f, 0.0f, 0.0f, 20.0f));
		
		RN::UI::TextField *alphaField = RN::UI::TextField::WithType(RN::UI::TextField::Type::Bezel)->Retain();
		alphaField->SetFormatter((new RN::NumberFormatter())->Autorelease());
		alphaField->SetValue(RN::Number::WithFloat(color.a));
		alphaField->SetFrame(RN::Rect(0.0f, 0.0f, 0.0f, 20.0f));
		
		auto applyChange = [=](RN::UI::Control *control, RN::UI::Control::EventType event){
			RN::Color color;
			color.r = redField->GetValue()->Downcast<RN::Number>()->GetFloatValue();
			color.g = greenField->GetValue()->Downcast<RN::Number>()->GetFloatValue();
			color.b = blueField->GetValue()->Downcast<RN::Number>()->GetFloatValue();
			color.a = alphaField->GetValue()->Downcast<RN::Number>()->GetFloatValue();
			setter(color);
		};
		
		redField->AddListener(RN::UI::Control::EventType::ValueChanged, applyChange, nullptr);
		greenField->AddListener(RN::UI::Control::EventType::ValueChanged, applyChange, nullptr);
		blueField->AddListener(RN::UI::Control::EventType::ValueChanged, applyChange, nullptr);
		alphaField->AddListener(RN::UI::Control::EventType::ValueChanged, applyChange, nullptr);
		
		
		_views->AddObject(RN::Array::WithObjects(label->Autorelease(), redLabel->Autorelease(), redField, greenLabel->Autorelease(), greenField, blueLabel->Autorelease(), blueField, alphaLabel->Autorelease(), alphaField, nullptr));
		
		AddSubview(label);
		AddSubview(redLabel);
		AddSubview(redField);
		AddSubview(greenLabel);
		AddSubview(greenField);
		AddSubview(blueLabel);
		AddSubview(blueField);
		AddSubview(alphaLabel);
		AddSubview(alphaField);
	}
	
	
	void MaterialView::InsertEnumWithTitle(RN::String *title, std::function<void (int32)> &&setter, int32 value, RN::UI::Menu *menu)
	{
		RN::UI::PopUpButton *popUpButton = new RN::UI::PopUpButton();
		popUpButton->SetFrame(RN::Rect(0.0f, 0.0f, 0.0f, 20.0f));
		popUpButton->SetMenu(menu);
		popUpButton->SetTitleColorForState(ColorScheme::GetUIColor(ColorScheme::Type::FileTree_Text), RN::UI::Control::State::Normal);
		
		menu->GetItems()->Enumerate<RN::UI::MenuItem>([popUpButton, value](RN::UI::MenuItem *item, size_t index, bool &stop){
			if(item->GetAssociatedObject(kDPMenuItemAssociatedKey)->Downcast<RN::Number>()->GetInt32Value() == value)
			{
				popUpButton->SetSelection(index);
				stop = true;
			}
		});
		
		popUpButton->AddListener(RN::UI::Control::EventType::ValueChanged, [=](RN::UI::Control *control, RN::UI::Control::EventType event) {
			RN::UI::PopUpButton *popUp = control->Downcast<RN::UI::PopUpButton>();
			setter(popUp->GetSelectedItem()->GetAssociatedObject(kDPMenuItemAssociatedKey)->Downcast<RN::Number>()->GetInt32Value());
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
