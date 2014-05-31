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
		
		
		RN::UI::Menu *polygonModeMenu = new RN::UI::Menu();
		polygonModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Points")));
		polygonModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Lines")));
		polygonModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Fill")));
		
		size_t polygonModeSelection = 0;
		if(material->GetPolygonMode() == RN::Material::PolygonMode::Lines)
			polygonModeSelection = 1;
		else if(material->GetPolygonMode() == RN::Material::PolygonMode::Fill)
			polygonModeSelection = 2;
		
		InsertEnumWithTitle(RNCSTR("Polygon mode"), [material](size_t value) {
			switch(value)
			{
				case 0:
					material->SetPolygonMode(RN::Material::PolygonMode::Points);
					break;
				case 1:
					material->SetPolygonMode(RN::Material::PolygonMode::Lines);
					break;
				case 2:
					material->SetPolygonMode(RN::Material::PolygonMode::Fill);
					break;
			}
		}, polygonModeSelection, polygonModeMenu->Autorelease());
		
		
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
		depthTestModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Never")));
		depthTestModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Always")));
		depthTestModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Less")));
		depthTestModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Less or Equal")));
		depthTestModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Equal")));
		depthTestModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Not Equal")));
		depthTestModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Greater or Equal")));
		depthTestModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Greater")));
		
		size_t depthTestModeSelection = 0;
		switch(material->GetDepthTestMode())
		{
			case RN::Material::DepthMode::Never:
				depthTestModeSelection = 0;
				break;
			case RN::Material::DepthMode::Always:
				depthTestModeSelection = 1;
				break;
			case RN::Material::DepthMode::Less:
				depthTestModeSelection = 2;
				break;
			case RN::Material::DepthMode::LessOrEqual:
				depthTestModeSelection = 3;
				break;
			case RN::Material::DepthMode::Equal:
				depthTestModeSelection = 4;
				break;
			case RN::Material::DepthMode::NotEqual:
				depthTestModeSelection = 5;
				break;
			case RN::Material::DepthMode::GreaterOrEqual:
				depthTestModeSelection = 6;
				break;
			case RN::Material::DepthMode::Greater:
				depthTestModeSelection = 7;
				break;
		}
		
		InsertEnumWithTitle(RNCSTR("Depth test mode"), [material](size_t value) {
			switch(value)
			{
				case 0:
					material->SetDepthTestMode(RN::Material::DepthMode::Never);
					break;
				case 1:
					material->SetDepthTestMode(RN::Material::DepthMode::Always);
					break;
				case 2:
					material->SetDepthTestMode(RN::Material::DepthMode::Less);
					break;
				case 3:
					material->SetDepthTestMode(RN::Material::DepthMode::LessOrEqual);
					break;
				case 4:
					material->SetDepthTestMode(RN::Material::DepthMode::Equal);
					break;
				case 5:
					material->SetDepthTestMode(RN::Material::DepthMode::NotEqual);
					break;
				case 6:
					material->SetDepthTestMode(RN::Material::DepthMode::GreaterOrEqual);
					break;
				case 7:
					material->SetDepthTestMode(RN::Material::DepthMode::Greater);
					break;
			}
		}, depthTestModeSelection, depthTestModeMenu->Autorelease());
		
		
		InsertBooleanWithTitle(RNCSTR("Double sided"), [material](bool value) {
			
			RN::Material::CullMode mode = value ? RN::Material::CullMode::None : RN::Material::CullMode::BackFace;
			material->SetCullMode(mode);
			
		}, (material->GetCullMode() == RN::Material::CullMode::None));
		
		InsertBooleanWithTitle(RNCSTR("Discard"), DPBindSetter(SetDiscard), material->GetDiscard());
		InsertFloatWithTitle(RNCSTR("Discard threshold"), DPBindSetter(SetDiscardThreshold), material->GetDiscardThreshold());
		
		InsertBooleanWithTitle(RNCSTR("Blending"), DPBindSetter(SetBlending), material->GetBlending());
		
		RN::UI::Menu *blendModeMenu = new RN::UI::Menu();
		blendModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Zero")));
		blendModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("One")));
		blendModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Source Color")));
		blendModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("One minus Source Color")));
		blendModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Source Alpha")));
		blendModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("One minus Source Alpha")));
		blendModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Destination Color")));
		blendModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("One minus Destination Color")));
		blendModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Destination Alpha")));
		blendModeMenu->AddItem(new RN::UI::MenuItem(RNCSTR("One minus Destination Alpha")));
		blendModeMenu->Autorelease();
		
		size_t blendSourceModeSelection = 0;
		switch(material->GetBlendSource())
		{
			case RN::Material::BlendMode::Zero:
				blendSourceModeSelection = 0;
				break;
			case RN::Material::BlendMode::One:
				blendSourceModeSelection = 1;
				break;
			case RN::Material::BlendMode::SourceColor:
				blendSourceModeSelection = 2;
				break;
			case RN::Material::BlendMode::OneMinusSourceColor:
				blendSourceModeSelection = 3;
				break;
			case RN::Material::BlendMode::SourceAlpha:
				blendSourceModeSelection = 4;
				break;
			case RN::Material::BlendMode::OneMinusSourceAlpha:
				blendSourceModeSelection = 5;
				break;
			case RN::Material::BlendMode::DestinationColor:
				blendSourceModeSelection = 6;
				break;
			case RN::Material::BlendMode::OneMinusDestinationColor:
				blendSourceModeSelection = 7;
				break;
			case RN::Material::BlendMode::DestinationAlpha:
				blendSourceModeSelection = 8;
				break;
			case RN::Material::BlendMode::OneMinusDestinationAlpha:
				blendSourceModeSelection = 9;
				break;
		}
		
		InsertEnumWithTitle(RNCSTR("Blend source mode"), [material](size_t value) {
			switch(value)
			{
				case 0:
					material->SetBlendMode(RN::Material::BlendMode::Zero, material->GetBlendDestination());
					break;
				case 1:
					material->SetBlendMode(RN::Material::BlendMode::One, material->GetBlendDestination());
					break;
				case 2:
					material->SetBlendMode(RN::Material::BlendMode::SourceColor, material->GetBlendDestination());
					break;
				case 3:
					material->SetBlendMode(RN::Material::BlendMode::OneMinusSourceColor, material->GetBlendDestination());
					break;
				case 4:
					material->SetBlendMode(RN::Material::BlendMode::SourceAlpha, material->GetBlendDestination());
					break;
				case 5:
					material->SetBlendMode(RN::Material::BlendMode::OneMinusSourceAlpha, material->GetBlendDestination());
					break;
				case 6:
					material->SetBlendMode(RN::Material::BlendMode::DestinationColor, material->GetBlendDestination());
					break;
				case 7:
					material->SetBlendMode(RN::Material::BlendMode::OneMinusDestinationColor, material->GetBlendDestination());
					break;
				case 8:
					material->SetBlendMode(RN::Material::BlendMode::DestinationAlpha, material->GetBlendDestination());
					break;
				case 9:
					material->SetBlendMode(RN::Material::BlendMode::OneMinusDestinationAlpha, material->GetBlendDestination());
					break;
			}
		}, blendSourceModeSelection, blendModeMenu);
		
		size_t blendDestinationModeSelection = 0;
		switch(material->GetBlendDestination())
		{
			case RN::Material::BlendMode::Zero:
				blendDestinationModeSelection = 0;
				break;
			case RN::Material::BlendMode::One:
				blendDestinationModeSelection = 1;
				break;
			case RN::Material::BlendMode::SourceColor:
				blendDestinationModeSelection = 2;
				break;
			case RN::Material::BlendMode::OneMinusSourceColor:
				blendDestinationModeSelection = 3;
				break;
			case RN::Material::BlendMode::SourceAlpha:
				blendDestinationModeSelection = 4;
				break;
			case RN::Material::BlendMode::OneMinusSourceAlpha:
				blendDestinationModeSelection = 5;
				break;
			case RN::Material::BlendMode::DestinationColor:
				blendDestinationModeSelection = 6;
				break;
			case RN::Material::BlendMode::OneMinusDestinationColor:
				blendDestinationModeSelection = 7;
				break;
			case RN::Material::BlendMode::DestinationAlpha:
				blendDestinationModeSelection = 8;
				break;
			case RN::Material::BlendMode::OneMinusDestinationAlpha:
				blendDestinationModeSelection = 9;
				break;
		}
		
		InsertEnumWithTitle(RNCSTR("Blend destination mode"), [material](size_t value) {
			switch(value)
			{
				case 0:
					material->SetBlendMode(material->GetBlendSource(), RN::Material::BlendMode::Zero);
					break;
				case 1:
					material->SetBlendMode(material->GetBlendSource(), RN::Material::BlendMode::One);
					break;
				case 2:
					material->SetBlendMode(material->GetBlendSource(), RN::Material::BlendMode::SourceColor);
					break;
				case 3:
					material->SetBlendMode(material->GetBlendSource(), RN::Material::BlendMode::OneMinusSourceColor);
					break;
				case 4:
					material->SetBlendMode(material->GetBlendSource(), RN::Material::BlendMode::SourceAlpha);
					break;
				case 5:
					material->SetBlendMode(material->GetBlendSource(), RN::Material::BlendMode::OneMinusSourceAlpha);
					break;
				case 6:
					material->SetBlendMode(material->GetBlendSource(), RN::Material::BlendMode::DestinationColor);
					break;
				case 7:
					material->SetBlendMode(material->GetBlendSource(), RN::Material::BlendMode::OneMinusDestinationColor);
					break;
				case 8:
					material->SetBlendMode(material->GetBlendSource(), RN::Material::BlendMode::DestinationAlpha);
					break;
				case 9:
					material->SetBlendMode(material->GetBlendSource(), RN::Material::BlendMode::OneMinusDestinationAlpha);
					break;
			}
		}, blendDestinationModeSelection, blendModeMenu);
		
		
		RN::UI::Menu *blendEquationMenu = new RN::UI::Menu();
		blendEquationMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Add")));
		blendEquationMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Substract")));
		blendEquationMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Reverse Substract")));
		blendEquationMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Min")));
		blendEquationMenu->AddItem(new RN::UI::MenuItem(RNCSTR("Max")));
		
		size_t blendEquationSelection = 0;
		switch(material->GetBlendEquation())
		{
			case RN::Material::BlendEquation::Add:
				blendEquationSelection = 0;
				break;
			case RN::Material::BlendEquation::Subtract:
				blendEquationSelection = 1;
				break;
			case RN::Material::BlendEquation::ReverseSubtract:
				blendEquationSelection = 2;
				break;
			case RN::Material::BlendEquation::Min:
				blendEquationSelection = 3;
				break;
			case RN::Material::BlendEquation::Max:
				blendEquationSelection = 4;
				break;
		}
		
		InsertEnumWithTitle(RNCSTR("Blend equation"), [material](size_t value) {
			switch(value)
			{
				case 0:
					material->SetBlendEquation(RN::Material::BlendEquation::Add);
					break;
				case 1:
					material->SetBlendEquation(RN::Material::BlendEquation::Subtract);
					break;
				case 2:
					material->SetBlendEquation(RN::Material::BlendEquation::ReverseSubtract);
					break;
				case 3:
					material->SetBlendEquation(RN::Material::BlendEquation::Min);
					break;
				case 4:
					material->SetBlendEquation(RN::Material::BlendEquation::Max);
					break;
			}
		}, blendEquationSelection, blendEquationMenu->Autorelease());
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
	
	
	void MaterialView::InsertEnumWithTitle(RN::String *title, std::function<void (size_t)> &&setter, size_t value, RN::UI::Menu *menu)
	{
		RN::UI::PopUpView *popUpView = new RN::UI::PopUpView();
		popUpView->SetFrame(RN::Rect(0.0f, 0.0f, 0.0f, 20.0f));
		popUpView->SetMenu(menu);
		popUpView->SetSelection(value);
		
		popUpView->AddListener(RN::UI::Control::EventType::ValueChanged, [=](RN::UI::Control *control, RN::UI::Control::EventType event) {
			RN::UI::PopUpView *popUp = control->Downcast<RN::UI::PopUpView>();
			setter(popUp->GetSelection());
		}, nullptr);
		
		InsertViewWithTitle(title, popUpView);
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
