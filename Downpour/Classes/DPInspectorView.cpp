//
//  DPInspectorView.cpp
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

#include "DPInspectorView.h"
#include "DPWorkspace.h"
#include "DPColorScheme.h"

namespace DP
{
	std::vector<std::pair<RN::MetaClass *, RN::MetaClass *>> *_registeredInspectorViews;
	
	RNDefineMeta(InspectorView, RN::UI::View)
	RNDefineMeta(GenericInspectorView, InspectorView)
	
	// -----------------------
	// MARK: -
	// MARK: InspectorViewContainer
	// -----------------------
	
	InspectorViewContainer::InspectorViewContainer() :
		_selection(nullptr),
		_inspectors(new RN::Array())
	{
		RN::MessageCenter::GetSharedInstance()->AddObserver(kDPWorkspaceSelectionChanged, [this](RN::Message *message) {
			SetSelection(message->GetObject());
		}, this);
	}
	
	InspectorViewContainer::~InspectorViewContainer()
	{
		RN::SafeRelease(_selection);
		RN::SafeRelease(_inspectors);
		
		RN::MessageCenter::GetSharedInstance()->RemoveObserver(this);
	}
	
	void InspectorViewContainer::SetSelection(RN::Object *object)
	{
		// The workspace uses arrays for selections, even when there is only one object
		// If there is only one object and it's a scene node, we will unbox the object from the array use that for the selection
		
		if(object && object->IsKindOfClass(RN::Array::GetMetaClass()) && static_cast<RN::Array *>(object)->GetCount() == 1)
		{
			RN::Array *array = static_cast<RN::Array *>(object);
			RN::Object *first = array->GetFirstObject();
			
			if(first->IsKindOfClass(RN::SceneNode::GetMetaClass()))
			{
				SetSelection(first);
				return;
			}
		}
		
		
		// Update the selection
		RN::SafeRelease(_selection);
		_selection = RN::SafeRetain(object);
		
		// Kick the old inspectors
		_inspectors->Enumerate<RN::UI::View>([&](RN::UI::View *view, size_t index, bool &stop) {
			view->RemoveFromSuperview();
		});
		_inspectors->RemoveAllObjects();
		
		if(!_selection)
			return;
		
		// Pick the right inspector views for the object
		std::vector<RN::MetaClass *> classes;
		
		RN::MetaClass *meta = _selection->GetClass();
		while(meta && meta != RN::Object::GetMetaClass())
		{
			classes.push_back(meta);
			meta = meta->GetSuperClass();
		}
		
		for(auto i = classes.rbegin(); i != classes.rend(); i ++)
		{
			meta = *i;
			
			RN::MetaClass *match = nullptr;
			size_t matchDistance = static_cast<size_t>(-1);
			
			for(auto i = _registeredInspectorViews->begin(); i != _registeredInspectorViews->end(); i ++)
			{
				auto &pair = *i;
				
				if(meta == pair.second)
				{
					match = pair.first;
					break;
				}
			}
			
			if(!match)
				match = GenericInspectorView::GetMetaClass();
			
			
			InspectorView *inspectorView = static_cast<InspectorView *>(match->Construct());
			
			inspectorView->Initialize(_selection, meta, RNSTR(meta->GetName().c_str()));
			inspectorView->SizeToFit();
			
			_inspectors->AddObject(inspectorView->Autorelease());
			AddSubview(inspectorView);
		}
	}
	
	void InspectorViewContainer::LayoutSubviews()
	{
		RN::UI::View::LayoutSubviews();
		
		float yoffset = 0.0f;
		float width   = GetBounds().width;
		
		_inspectors->Enumerate<RN::UI::View>([&](RN::UI::View *view, size_t index, bool &stop) {
			
			RN::Rect frame = RN::Rect(0.0f, yoffset, width, view->GetSizeThatFits().y);
			view->SetFrame(frame);
			
			yoffset += frame.height;
			
		});
		
		SetContentSize(RN::Vector2(GetBounds().width, yoffset));
	}
	
	// -----------------------
	// MARK: -
	// MARK: InspectorView
	// -----------------------
	
	void InspectorView::RegisterInspectorViewForClass(RN::MetaClass *inspectorClass, RN::MetaClass *predicate)
	{
		RN_ASSERT(inspectorClass->InheritsFromClass(InspectorView::GetMetaClass()), "Inspector class must inherit from InspectorView");
		
		static std::once_flag token;
		std::call_once(token, [&] {
			_registeredInspectorViews = new std::vector<std::pair<RN::MetaClass *, RN::MetaClass *>>();
		});
		
		_registeredInspectorViews->emplace_back(std::make_pair(inspectorClass, predicate));
	}
	
	
	InspectorView::InspectorView() :
		_object(nullptr),
		_meta(nullptr),
		_propertyViews(new RN::Array())
	{
		_titleLabel = new RN::UI::Label();
		_titleLabel->SetTextColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_Text));
		_titleLabel->SetFont(RN::UI::Style::GetSharedInstance()->GetFont(RN::UI::Style::FontStyle::DefaultFontBold));
		_titleLabel->SetFrame([&]() -> RN::Rect {
			RN::Rect rect = _titleLabel->GetFrame();
			rect.x = 5.0f;
			rect.y = 5.0f;
			
			return rect;
		}());
		
		AddSubview(_titleLabel);
	}
	
	InspectorView::~InspectorView()
	{
		_titleLabel->Release();
		_propertyViews->Release();
	}
	
	void InspectorView::Initialize(RN::Object *object, RN::MetaClass *meta, RN::String *title)
	{
		_object = object;
		_meta   = meta;
		
		_titleLabel->SetText(title);
		_titleLabel->SizeToFit();
	}
	
	
	void InspectorView::AddPropertyView(PropertyView *view)
	{
		if(!view)
			return;
		
		AddSubview(view);
		_propertyViews->AddObject(view);
	}
	
	RN::Vector2 InspectorView::GetSizeThatFits()
	{
		float height = 13.0f + _titleLabel->GetBounds().height;
		float width  = GetBounds().width;
		
		_propertyViews->Enumerate<RN::UI::View>([&](RN::UI::View *view, size_t index, bool &stop) {
			
			RN::Rect frame = view->GetFrame();
			height += frame.height + 2.0f;
			
		});
		
		return RN::Vector2(width, height);
	}
	
	void InspectorView::LayoutSubviews()
	{
		RN::UI::View::LayoutSubviews();
		
		float yoffset = 8.0f + _titleLabel->GetBounds().height;
		float width   = GetBounds().width;
		
		_propertyViews->Enumerate<RN::UI::View>([&](RN::UI::View *view, size_t index, bool &stop) {
			
			RN::Rect frame = view->GetFrame();
			
			frame.x = 0.0f;
			frame.y = yoffset;
			frame.width = width;
			
			view->SetFrame(frame);
			
			yoffset += frame.height + 2.0f;
			
		});
	}
	
	// -----------------------
	// MARK: -
	// MARK: InspectorView
	// -----------------------
	
	GenericInspectorView::GenericInspectorView()
	{}
	
	void GenericInspectorView::Initialize(RN::Object *object, RN::MetaClass *meta, RN::String *title)
	{
		InspectorView::Initialize(object, meta, title);
		
		std::vector<RN::ObservableProperty *> properties = object->GetPropertiesForClass(meta);
		
		for(RN::ObservableProperty *property : properties)
		{
			RN::String *name = RNSTR(property->GetName().c_str());
			name->Capitalize();
			
			AddPropertyView(PropertyView::WithObservable(property, name));
		}
	}
	
	void GenericInspectorView::InitialWakeUp(RN::MetaClass *meta)
	{
		if(meta == GenericInspectorView::GetMetaClass())
		{
			RegisterInspectorViewForClass(meta, RN::Object::GetMetaClass());
		}
	}
}
