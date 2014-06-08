//
//  DPSculptableInspectorView.cpp
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

#include "DPSculptableInspectorView.h"

namespace DP
{
	RNDefineMeta(SculptableInspectorView, InspectorView)
	
	SculptableInspectorView::SculptableInspectorView()
	{
		_previousTool = Workspace::GetSharedInstance()->GetActiveTool();
	}
	
	SculptableInspectorView::~SculptableInspectorView()
	{
		Workspace::GetSharedInstance()->SetActiveTool(_previousTool);
	}
	
	void SculptableInspectorView::Initialize(RN::Object *object, RN::MetaClass *meta, RN::String *title)
	{
		InspectorView::Initialize(object, meta, title);
		
		// Sculpting on/off
		RN::UI::Button *sculptButton = new RN::UI::Button(RN::UI::Button::Type::Bezel);
		sculptButton->SetFrame(RN::Rect(10.0f, 5.0f, 80.0f, 30.0f));
		sculptButton->SetBehavior(RN::UI::Button::Behavior::Switch);
		sculptButton->SetTitleForState(RNCSTR("Off"), RN::UI::Control::State::Normal);
		sculptButton->SetTitleForState(RNCSTR("On"), RN::UI::Control::State::Selected);
		sculptButton->SetTitleColorForState(ColorScheme::GetUIColor(ColorScheme::Type::FileTree_Text), RN::UI::Control::State::Normal);
		sculptButton->SetFontForState(RN::UI::Style::GetSharedInstance()->GetFont(RN::UI::Style::FontStyle::DefaultFontBold), RN::UI::Control::State::Normal);
		sculptButton->AddListener(RN::UI::Control::EventType::MouseUpInside, [this, object](RN::UI::Control *control, RN::UI::Control::EventType event) {
			
			RN::Sculptable *node = object->Downcast<RN::Sculptable>();
			if(control->IsSelected() && node)
			{
				if(Workspace::GetSharedInstance()->GetActiveTool() != DP::Workspace::Tool::Sculpting)
				{
					_previousTool = Workspace::GetSharedInstance()->GetActiveTool();
					Workspace::GetSharedInstance()->SetActiveTool(DP::Workspace::Tool::Sculpting);
				}
			}
			else
			{
				Workspace::GetSharedInstance()->SetActiveTool(_previousTool);
			}
			
		}, this);
		
		PropertyView *sculptProperty = new PropertyView(RNSTR("Sculpting: "), DP::PropertyView::Layout::TitleLeft);
		sculptProperty->GetContentView()->AddSubview(sculptButton);
		sculptProperty->SetPreferredHeight(40.0f);
		AddPropertyView(sculptProperty);
		
		//Shape sphere/cube
		RN::UI::Button *shapeButton = new RN::UI::Button(RN::UI::Button::Type::Bezel);
		shapeButton->SetFrame(RN::Rect(10.0f, 5.0f, 80.0f, 30.0f));
		shapeButton->SetBehavior(RN::UI::Button::Behavior::Switch);
		shapeButton->SetTitleForState(RNCSTR("Sphere"), RN::UI::Control::State::Normal);
		shapeButton->SetTitleForState(RNCSTR("Cube"), RN::UI::Control::State::Selected);
		shapeButton->SetTitleColorForState(ColorScheme::GetUIColor(ColorScheme::Type::FileTree_Text), RN::UI::Control::State::Normal);
		shapeButton->SetFontForState(RN::UI::Style::GetSharedInstance()->GetFont(RN::UI::Style::FontStyle::DefaultFontBold), RN::UI::Control::State::Normal);
		shapeButton->AddListener(RN::UI::Control::EventType::MouseUpInside, [this, object](RN::UI::Control *control, RN::UI::Control::EventType event) {
			
			RN::Sculptable *node = object->Downcast<RN::Sculptable>();
			if(control->IsSelected() && node)
			{
				Workspace::GetSharedInstance()->GetSculptTool()->SetShape(SculptTool::Shape::Cube);
			}
			else
			{
				Workspace::GetSharedInstance()->GetSculptTool()->SetShape(SculptTool::Shape::Sphere);
			}
			
		}, this);
		
		PropertyView *shapeProperty = new PropertyView(RNSTR("Shape: "), DP::PropertyView::Layout::TitleLeft);
		shapeProperty->GetContentView()->AddSubview(shapeButton);
		shapeProperty->SetPreferredHeight(40.0f);
		AddPropertyView(shapeProperty);
		
		//mode add/substract
		RN::UI::Button *modeButton = new RN::UI::Button(RN::UI::Button::Type::Bezel);
		modeButton->SetFrame(RN::Rect(10.0f, 5.0f, 80.0f, 30.0f));
		modeButton->SetBehavior(RN::UI::Button::Behavior::Switch);
		modeButton->SetTitleForState(RNCSTR("Add"), RN::UI::Control::State::Normal);
		modeButton->SetTitleForState(RNCSTR("Substract"), RN::UI::Control::State::Selected);
		modeButton->SetTitleColorForState(ColorScheme::GetUIColor(ColorScheme::Type::FileTree_Text), RN::UI::Control::State::Normal);
		modeButton->SetFontForState(RN::UI::Style::GetSharedInstance()->GetFont(RN::UI::Style::FontStyle::DefaultFontBold), RN::UI::Control::State::Normal);
		modeButton->AddListener(RN::UI::Control::EventType::MouseUpInside, [this, object](RN::UI::Control *control, RN::UI::Control::EventType event) {
			
			RN::Sculptable *node = object->Downcast<RN::Sculptable>();
			if(control->IsSelected() && node)
			{
				Workspace::GetSharedInstance()->GetSculptTool()->SetMode(SculptTool::Mode::Substract);
			}
			else
			{
				Workspace::GetSharedInstance()->GetSculptTool()->SetMode(SculptTool::Mode::Add);
			}
			
		}, this);
		
		PropertyView *modeProperty = new PropertyView(RNSTR("Mode: "), DP::PropertyView::Layout::TitleLeft);
		modeProperty->GetContentView()->AddSubview(modeButton);
		modeProperty->SetPreferredHeight(40.0f);
		AddPropertyView(modeProperty);
		
		SculptTool *sculptTool = Workspace::GetSharedInstance()->GetSculptTool();
		
		std::vector<RN::ObservableProperty *> properties = object->GetPropertiesForClass(meta);
		std::vector<RN::ObservableProperty *> sculptProperties = sculptTool->GetPropertiesForClass(SculptTool::GetMetaClass());
		properties.insert(properties.begin(), sculptProperties.begin(), sculptProperties.end());
		
		for(RN::ObservableProperty *property : properties)
		{
			RN::String *name = RNSTR(property->GetName().c_str());
			name->Capitalize();
			
			AddPropertyView(PropertyView::WithObservable(property, name));
		}
	}
	
	void SculptableInspectorView::InitialWakeUp(RN::MetaClass *meta)
	{
		if(meta == SculptableInspectorView::GetMetaClass())
		{
			RegisterInspectorViewForClass(meta, RN::Sculptable::GetMetaClass());
		}
	}
}
