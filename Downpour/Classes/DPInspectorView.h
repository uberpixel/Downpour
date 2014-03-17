//
//  DPInspectorView.h
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

#ifndef __DPINSPECTORVIEW_H__
#define __DPINSPECTORVIEW_H__

#include <Rayne/Rayne.h>
#include "DPPropertyView.h"

namespace DP
{
	class InspectorViewContainer : public RN::UI::ScrollView
	{
	public:
		InspectorViewContainer();
		~InspectorViewContainer();
		
		void SetSelection(RN::Object *object);
		
		void LayoutSubviews() override;
		
	private:
		RN::Object *_selection;
		RN::Array *_inspectors;
	};
	
	class InspectorView : public RN::UI::View
	{
	public:
		~InspectorView();
		
		RN::Object *GetObject() const { return _object; }
		RN::MetaClassBase *GetMetaClassBase() const { return _meta; }
		
		RN::Vector2 GetSizeThatFits() override;
		
		void LayoutSubviews() override;
		
	protected:
		InspectorView(RN::Object *object, RN::MetaClassBase *meta, RN::String *title);
		
		void AddPropertyView(PropertyView *view);
		
	private:
		RN::Object *_object;
		RN::MetaClassBase *_meta;
		
		RN::UI::Label *_titleLabel;
		RN::Array *_propertyViews;
	};
	
	class GenericInspectorView : public InspectorView
	{
	public:
		GenericInspectorView(RN::Object *object, RN::MetaClassBase *meta, RN::String *title);
	};
}

#endif /* __DPINSPECTORVIEW_H__ */
