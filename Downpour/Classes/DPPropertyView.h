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
#include "DPDragNDropTarget.h"

extern const char *kDPEnumItemAssociatedKey;

#define DPMenuItemWithTitleAndObject(title, object) \
	[]() -> RN::UI::MenuItem* { \
		RN::UI::MenuItem *temp = RN::UI::MenuItem::WithTitle(title); \
		temp->SetAssociatedObject(kDPEnumItemAssociatedKey, object, RN::Object::MemoryPolicy::Retain); \
		return temp; \
	}()

namespace DP
{
	class PropertyView : public RN::UI::View
	{
	public:
		friend class InspectorView;
		
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
		void LayoutContentView();
		
		RN::UI::Label *_titleLabel;
		RN::UI::View *_contentView;
		
		Layout _layout;
		float _preferredHeight;
		
		RNDeclareMeta(PropertyView)
	};
	
	class ObservablePropertyView : public PropertyView
	{
	public:
		ObservablePropertyView(RN::ObservableProperty *observable, RN::String *title, PropertyView::Layout layout);
		~ObservablePropertyView();
		
		virtual void ValueDidChange(RN::Object *value) = 0;
		
	protected:
		RN::ObservableProperty *_observable;
		
		RNDeclareMeta(ObservablePropertyView)
	};
	
	class BooleanPropertyView : public ObservablePropertyView
	{
	public:
		BooleanPropertyView(RN::ObservableProperty *observable, RN::String *title);
		
		void ValueDidChange(RN::Object *value) override;
		
	private:
		void ButtonClicked();
		
		RN::UI::Button *_valueButton;
		
		RNDeclareMeta(ObservablePropertyView)
	};
	
	class ScalarPropertyView : public ObservablePropertyView, RN::UI::TextFieldDelegate
	{
	public:
		ScalarPropertyView(RN::ObservableProperty *observable, RN::String *title);
		
	private:
		void TextFieldDidEndEditing(RN::UI::TextField *textField) override;
		void ValueDidChange(RN::Object *value) override;
		
		RN::UI::TextField *_valueField;
		
		RNDeclareMeta(ScalarPropertyView)
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
		
		RNDeclareMeta(ComponentPropertyView)
	};
	
	class Vector2PropertyView : public ComponentPropertyView
	{
	public:
		Vector2PropertyView(RN::ObservableProperty *observable, RN::String *title);
		
		void ValueAtIndexChanged(size_t index) override;
		void ValueDidChange(RN::Object *newValue) override;
		
	private:
		RNDeclareMeta(Vector2PropertyView)
	};
	
	class Vector3PropertyView : public ComponentPropertyView
	{
	public:
		Vector3PropertyView(RN::ObservableProperty *observable, RN::String *title);
		
		void ValueAtIndexChanged(size_t index) override;
		void ValueDidChange(RN::Object *newValue) override;
		
	private:
		RNDeclareMeta(Vector3PropertyView)
	};
	
	class QuaternionPropertyView : public ComponentPropertyView
	{
	public:
		QuaternionPropertyView(RN::ObservableProperty *observable, RN::String *title);
		
		void ValueAtIndexChanged(size_t index) override;
		void ValueDidChange(RN::Object *newValue) override;
		
	private:
		RNDeclareMeta(QuaternionPropertyView)
	};
	
	class ColorPropertyView : public ObservablePropertyView
	{
	public:
		ColorPropertyView(RN::ObservableProperty *observable, RN::String *title);
		
		void ValueDidChange(RN::Object *newValue) override;
		
	private:
		RN::UI::ColorView *_colorView;
		
		RNDeclareMeta(ColorPropertyView)
	};
	
	class ModelPropertyView : public ObservablePropertyView, DelegatingDragNDropTarget::Delegate
	{
	public:
		ModelPropertyView(RN::ObservableProperty *observable, RN::String *title);
		~ModelPropertyView();
		
		void ValueDidChange(RN::Object *value) override;
		void LayoutSubviews() override;
		
		bool DragNDropTargetAcceptsDropOfObject(DelegatingDragNDropTarget *target, RN::Object *object) override;
		void DragNDropTargetHandleDropOfObject(DelegatingDragNDropTarget *target, RN::Object *object, const RN::Vector2 &position) override;
		
	private:
		RN::UI::Label *_infoLabel;
		RN::Array *_meshes;
		
		DelegatingDragNDropTarget *_dragTarget;
		
		RNDeclareMeta(ModelPropertyView)
	};
	
	class EnumPropertyView : public PropertyView
	{
	public:
		EnumPropertyView(RN::UI::Menu *menu, RN::String *title, std::function<void (int32)> &&setter, int32 value);
		~EnumPropertyView();
		
		void LayoutSubviews() override;
		
	private:
		RN::UI::PopUpButton *_popUpButton;
		
		RNDeclareMeta(EnumPropertyView)
	};
}

#endif /* __DPPROPERTYVIEW_H__ */
