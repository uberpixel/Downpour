//
//  DPLightInspectorView.cpp
//  Downpour
//
//  Created by Nils Daumann on 08.06.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
//

#include "DPLightInspectorView.h"

namespace DP
{
	RNDefineMeta(LightInspectorView, InspectorView)
	
	LightInspectorView::LightInspectorView()
	{
		
	}
	
	LightInspectorView::~LightInspectorView()
	{
		
	}
	
	void LightInspectorView::Initialize(RN::Object *object, RN::MetaClass *meta, RN::String *title)
	{
		InspectorView::Initialize(object, meta, title);
		
		RN::Light *light = object->Downcast<RN::Light>();
		
		// List the lights observables
		std::vector<RN::ObservableProperty *> properties = object->GetPropertiesForClass(meta);
		for(RN::ObservableProperty *property : properties)
		{
			RN::String *name = RNSTR(property->GetName().c_str());
			name->Capitalize();
			
			AddPropertyView(PropertyView::WithObservable(property, name));
		}
		
		RN::UI::Menu *typeMenu = new RN::UI::Menu();
		typeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Point Light"), RN::Number::WithInt32(static_cast<int32>(RN::Light::Type::PointLight))));
		
		typeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Spot Light"), RN::Number::WithInt32(static_cast<int32>(RN::Light::Type::SpotLight))));
		
		typeMenu->AddItem(DPMenuItemWithTitleAndObject(RNCSTR("Directional Light"), RN::Number::WithInt32(static_cast<int32>(RN::Light::Type::DirectionalLight))));
		
		PropertyView *typeProperty = new EnumPropertyView(typeMenu, RNCSTR("Type"), [light](int32 value) {
			light->SetType(static_cast<RN::Light::Type>(value));
		}, static_cast<int32>(light->GetType()));
		AddPropertyView(typeProperty->Autorelease());
	}
	
	void LightInspectorView::InitialWakeUp(RN::MetaClass *meta)
	{
		if(meta == LightInspectorView::GetMetaClass())
		{
			RegisterInspectorViewForClass(meta, RN::Light::GetMetaClass());
		}
	}
}
