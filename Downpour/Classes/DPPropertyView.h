//
//  DPPropertyView.h
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

#ifndef __DPPROPERTYVIEW_H__
#define __DPPROPERTYVIEW_H__

#include <Rayne/Rayne.h>

namespace DP
{
	class PropertyView : public RN::UI::View
	{
	public:
		enum class Layout
		{
			TitleTop,
			TitleLeft
		};
		
		PropertyView(RN::String *title, Layout layout);
		~PropertyView();
		
		void SetPreferredHeight(float height);
		void LayoutSubviews() override;
		
		RN::UI::View *GetContentView() const { return _contentView; }
		
		static PropertyView *WithObservable(RN::ObservableProperty *observable, RN::String *title);
		
	private:
		RN::UI::Label *_titleLabel;
		RN::UI::View *_contentView;
		
		Layout _layout;
		
		RNDeclareMeta(PropertyView, RN::UI::View)
	};
	
	class ObservablePropertyView : public PropertyView
	{
	public:
		ObservablePropertyView(RN::ObservableProperty *observable, RN::String *title, PropertyView::Layout layout);
		~ObservablePropertyView();
		
		virtual void ValueDidChange(RN::Object *value) = 0;
		
	protected:
		RN::ObservableProperty *_observable;
		
		RNDeclareMeta(ObservablePropertyView, PropertyView)
	};
	
	class ScalarPropertyView : public ObservablePropertyView, RN::UI::TextFieldDelegate
	{
	public:
		ScalarPropertyView(RN::ObservableProperty *observable, RN::String *title);
		
	private:
		void TextFieldDidEndEditing(RN::UI::TextField *textField) override;
		void ValueDidChange(RN::Object *value) override;
		
		RN::UI::TextField *_valueField;
		
		RNDeclareMeta(ScalarPropertyView, ObservablePropertyView)
	};
	
	class ComponentPropertyView : public ObservablePropertyView, RN::UI::TextFieldDelegate
	{
	public:
		ComponentPropertyView(RN::ObservableProperty *observable, RN::String *title, size_t components);
		
		void SetTitle(RN::String *text, size_t index);
		void SetValue(RN::Object *value, size_t index);
		RN::Object *GetValue(size_t index);
		
		virtual void ValueAtIndexChanged(size_t index) = 0;
		
		void LayoutSubviews() override;
		
	private:
		void TextFieldDidEndEditing(RN::UI::TextField *textField) override;
		void LayoutComponent(RN::UI::Label *label, RN::UI::TextField *value, float offset, float size);
		
		std::vector<std::pair<RN::UI::Label *, RN::UI::TextField *>> _components;
		
		RNDeclareMeta(ComponentPropertyView, ObservablePropertyView)
	};
	
	class Vector3PropertyView : public ComponentPropertyView
	{
	public:
		Vector3PropertyView(RN::ObservableProperty *observable, RN::String *title);
		
		void ValueAtIndexChanged(size_t index) override;
		void ValueDidChange(RN::Object *newValue) override;
		
	private:
		RNDeclareMeta(Vector3PropertyView, ComponentPropertyView)
	};
	
	class QuaternionPropertyView : public ComponentPropertyView
	{
	public:
		QuaternionPropertyView(RN::ObservableProperty *observable, RN::String *title);
		
		void ValueAtIndexChanged(size_t index) override;
		void ValueDidChange(RN::Object *newValue) override;
		
	private:
		RNDeclareMeta(QuaternionPropertyView, ComponentPropertyView)
	};
}

#endif /* __DPPROPERTYVIEW_H__ */