//
//  DPSculptableInspectorView.cpp
//  Downpour
//
//  Created by Nils Daumann on 06.04.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
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
	
	void SculptableInspectorView::Initialize(RN::Object *object, RN::MetaClassBase *meta, RN::String *title)
	{
		InspectorView::Initialize(object, meta, title);
		
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
		std::vector<RN::ObservableProperty *> sculptProperties = sculptTool->GetPropertiesForClass(SculptTool::MetaClass());
		properties.insert(properties.begin(), sculptProperties.begin(), sculptProperties.end());
		
		for(RN::ObservableProperty *property : properties)
		{
			RN::String *name = RNSTR(property->GetName().c_str());
			name->Capitalize();
			
			AddPropertyView(PropertyView::WithObservable(property, name));
		}
	}
	
	void SculptableInspectorView::InitialWakeUp(RN::MetaClassBase *meta)
	{
		if(meta == SculptableInspectorView::MetaClass())
		{
			RegisterInspectorViewForClass(meta, RN::Sculptable::MetaClass());
		}
	}
}