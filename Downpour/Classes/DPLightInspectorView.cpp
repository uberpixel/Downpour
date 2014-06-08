//
//  DPLightInspectorView.cpp
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
