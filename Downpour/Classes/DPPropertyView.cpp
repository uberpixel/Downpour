//
//  DPPropertyView.cpp
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

#include "DPPropertyView.h"
#include "DPColorScheme.h"

#define kDPPropertyViewLayoutLeftTitleLength 65.0f

namespace DP
{
	RNDefineMeta(PropertyView, RN::UI::View)
	RNDefineMeta(ObservablePropertyView, PropertyView)
	RNDefineMeta(ScalarPropertyView, ObservablePropertyView)
	RNDefineMeta(ComponentPropertyView, ObservablePropertyView)
	RNDefineMeta(Vector3PropertyView, ComponentPropertyView)
	RNDefineMeta(QuaternionPropertyView, ComponentPropertyView)
	RNDefineMeta(ColorPropertyView, ComponentPropertyView)
	
	// -----------------------
	// MARK: -
	// MARK: PropertyView
	// -----------------------
	
	PropertyView::PropertyView(RN::String *title, Layout layout) :
		_layout(layout)
	{
		_titleLabel = new RN::UI::Label();
		_titleLabel->SetText(title);
		_titleLabel->SetTextColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_Text));
		_titleLabel->SizeToFit();
		_titleLabel->SetFrame([&]() -> RN::Rect {
			RN::Rect rect = _titleLabel->GetFrame();
			rect.x = 5.0f;
			rect.y = 2.0f;
			
			if(layout == Layout::TitleLeft)
			{
				rect.width = kDPPropertyViewLayoutLeftTitleLength;
				_titleLabel->SetAlignment(RN::UI::TextAlignment::Right);
			}
			
			return rect;
		}());
		
		_contentView = new RN::UI::View();
		
		AddSubview(_titleLabel);
		AddSubview(_contentView);
	}
	
	PropertyView::~PropertyView()
	{
		_titleLabel->Release();
		_contentView->Release();
	}
	
	void PropertyView::SetPreferredHeight(float height)
	{
		RN::Rect frame = GetBounds();
		
		if(_layout == Layout::TitleTop)
		{
			frame.height = height + 6.0f; // 2 points padding top and bottom + 2 points padding between content and title
			frame.height += _titleLabel->GetBounds().height;
		}
		else
		{
			height = std::max(_titleLabel->GetBounds().height, height);
			frame.height = height + 4.0f;
		}
		
		SetFrame(frame);
	}
	
	void PropertyView::LayoutSubviews()
	{
		RN::UI::View::LayoutSubviews();
		
		RN::Rect frame = GetBounds();
		frame.Inset(5.0f, 2.0f);
		
		if(_layout == Layout::TitleTop)
		{
			float titleHeight = _titleLabel->GetBounds().height;
			
			frame.y      += 2.0f + titleHeight;
			frame.height -= 2.0f + titleHeight;
		}
		else
		{
			frame.x    += 5.0f + kDPPropertyViewLayoutLeftTitleLength;
			frame.width = frame.width - (kDPPropertyViewLayoutLeftTitleLength + 5.0f);
		}
		
		_contentView->SetFrame(frame);
	}
	
	PropertyView *PropertyView::WithObservable(RN::ObservableProperty *observable, RN::String *title)
	{
		switch(observable->GetType())
		{
			case RN::TypeTranslator<int8>::value:
			case RN::TypeTranslator<int16>::value:
			case RN::TypeTranslator<int32>::value:
			case RN::TypeTranslator<int64>::value:
			case RN::TypeTranslator<uint8>::value:
			case RN::TypeTranslator<uint16>::value:
			case RN::TypeTranslator<uint32>::value:
			case RN::TypeTranslator<uint64>::value:
			case RN::TypeTranslator<float>::value:
			case RN::TypeTranslator<double>::value:
			{
				ScalarPropertyView *view = new ScalarPropertyView(observable, title);
				return view->Autorelease();
			}
				
			case RN::TypeTranslator<RN::Vector3>::value:
			{
				Vector3PropertyView *view = new Vector3PropertyView(observable, title);
				return view->Autorelease();
			}
				
			case RN::TypeTranslator<RN::Quaternion>::value:
			{
				QuaternionPropertyView *view = new QuaternionPropertyView(observable, title);
				return view->Autorelease();
			}
				
			case RN::TypeTranslator<RN::Color>::value:
			{
				ColorPropertyView *view = new ColorPropertyView(observable, title);
				return view->Autorelease();
			}
				
			default:
				return nullptr;
		}
	}
	
	// -----------------------
	// MARK: -
	// MARK: ObservablePropertyView
	// -----------------------
	
	ObservablePropertyView::ObservablePropertyView(RN::ObservableProperty *observable, RN::String *title, PropertyView::Layout layout) :
		PropertyView(title, layout),
		_observable(observable)
	{
		RN::Object *object = observable->GetObject();
		
		object->AddObserver(observable->GetName(), [this](RN::Object *object, const std::string &key, RN::Dictionary *changes) {
			
			RN::Object *value = changes->GetObjectForKey(kRNObservableNewValueKey);
			ValueDidChange(value);
			
		}, this);
	}
	
	ObservablePropertyView::~ObservablePropertyView()
	{
		RN::Object *object = _observable->GetObject();
		object->RemoveObserver(_observable->GetName(), this);
	}
	
	// -----------------------
	// MARK: -
	// MARK: ScalarPropertyView
	// -----------------------
	
	ScalarPropertyView::ScalarPropertyView(RN::ObservableProperty *observable, RN::String *title) :
		ObservablePropertyView(observable, title, PropertyView::Layout::TitleLeft)
	{
		_valueField = RN::UI::TextField::WithType(RN::UI::TextField::Type::Bezel)->Retain();
		_valueField->SetFormatter((new RN::NumberFormatter())->Autorelease());
		_valueField->SetAutoresizingMask(RN::UI::View::AutoresizingFlexibleWidth);
		_valueField->SetDelegate(this);
		_valueField->SetValue(observable->GetValue());
		_valueField->SetFrame([&]() -> RN::Rect {
			
			RN::Rect frame = _valueField->GetFrame();
			frame.height = 20.0f;
			frame.width  = GetContentView()->GetBounds().width;
			
			return frame;
			
		}());
		
		GetContentView()->AddSubview(_valueField);
		SetPreferredHeight(20.0f);
	}
	
	void ScalarPropertyView::TextFieldDidEndEditing(RN::UI::TextField *textField)
	{
		_observable->SetValue(textField->GetValue());
	}
	
	void ScalarPropertyView::ValueDidChange(RN::Object *value)
	{
		_valueField->SetValue(value);
	}
	
	// -----------------------
	// MARK: -
	// MARK: ComponentPropertyView
	// -----------------------
	
	ComponentPropertyView::ComponentPropertyView(RN::ObservableProperty *observable, RN::String *title, size_t components)  :
		ObservablePropertyView(observable, title, PropertyView::Layout::TitleTop)
	{
		RN::NumberFormatter *formatter = new RN::NumberFormatter();
		
		for(size_t i = 0; i < components; i ++)
		{
			RN::UI::Label *label = new RN::UI::Label();
			RN::UI::TextField *value = RN::UI::TextField::WithType(RN::UI::TextField::Type::Bezel);
			
			label->SetTextColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_Text));
			
			value->SetFormatter(formatter);
			value->SetDelegate(this);
			
			GetContentView()->AddSubview(label->Autorelease());
			GetContentView()->AddSubview(value);
			
			_components.push_back(std::make_pair(label, value));
		}
		
		formatter->Release();
		SetPreferredHeight(20.0f);
	}
	
	
	void ComponentPropertyView::SetTitle(RN::String *text, size_t index)
	{
		_components[index].first->SetText(text);
	}
	
	void ComponentPropertyView::SetValue(RN::Object *value, size_t index)
	{
		_components[index].second->SetValue(value);
	}
	
	RN::Object *ComponentPropertyView::GetValue(size_t index)
	{
		auto &pair = _components.at(index);
		return pair.second->GetValue();
	}
	
	
	void ComponentPropertyView::TextFieldDidEndEditing(RN::UI::TextField *textField)
	{
		for(size_t i = 0; i < _components.size(); i ++)
		{
			if(_components[i].second == textField)
			{
				ValueAtIndexChanged(i);
				return;
			}
		}
	}
	
	void ComponentPropertyView::LayoutSubviews()
	{
		PropertyView::LayoutSubviews();
		
		float offset = 0.0f;
		float width  = GetContentView()->GetBounds().width;
		
		float cellSize = roundf(width / _components.size());
		
		for(size_t i = 0; i < _components.size(); i ++)
		{
			auto &pair = _components[i];
			LayoutComponent(pair.first, pair.second, offset + (i * cellSize), cellSize - 2.0f);
		}
	}
	
	void ComponentPropertyView::LayoutComponent(RN::UI::Label *label, RN::UI::TextField *value, float offset, float size)
	{
		float labelWidth = label->GetSizeThatFits().x;
		
		label->SetFrame(RN::Rect(offset, 2.0f, labelWidth, 18.0f));
		value->SetFrame(RN::Rect(offset + labelWidth + 2.0f, 0.0f, size - labelWidth - 2.0f, 20.0f));
	}
	
	// -----------------------
	// MARK: -
	// MARK: Vector3ComponentView
	// -----------------------
	
	Vector3PropertyView::Vector3PropertyView(RN::ObservableProperty *observable, RN::String *title) :
		ComponentPropertyView(observable, title, 3)
	{
		SetTitle(RNCSTR("X"), 0);
		SetTitle(RNCSTR("Y"), 1);
		SetTitle(RNCSTR("Z"), 2);
		
		ValueDidChange(_observable->GetValue());
	}
	
	void Vector3PropertyView::ValueAtIndexChanged(size_t index)
	{
		RN::Vector3 vector;
		
		vector.x = GetValue(0)->Downcast<RN::Number>()->GetFloatValue();
		vector.y = GetValue(1)->Downcast<RN::Number>()->GetFloatValue();
		vector.z = GetValue(2)->Downcast<RN::Number>()->GetFloatValue();
		
		_observable->SetValue(RN::Value::WithVector3(vector));
	}
	
	void Vector3PropertyView::ValueDidChange(RN::Object *newValue)
	{
		RN::Value *value = newValue->Downcast<RN::Value>();
		RN::Vector3 vector = value->GetValue<RN::Vector3>();
		
		SetValue(RN::Number::WithFloat(vector.x), 0);
		SetValue(RN::Number::WithFloat(vector.y), 1);
		SetValue(RN::Number::WithFloat(vector.z), 2);
	}
	
	// -----------------------
	// MARK: -
	// MARK: QuaternionPropertyView
	// -----------------------
	
	QuaternionPropertyView::QuaternionPropertyView(RN::ObservableProperty *observable, RN::String *title) :
		ComponentPropertyView(observable, title, 3)
	{
		SetTitle(RNCSTR("X"), 0);
		SetTitle(RNCSTR("Y"), 1);
		SetTitle(RNCSTR("Z"), 2);
		
		ValueDidChange(_observable->GetValue());
	}
	
	void QuaternionPropertyView::ValueAtIndexChanged(size_t index)
	{
		RN::Vector3 vector;
		
		vector.x = GetValue(0)->Downcast<RN::Number>()->GetFloatValue();
		vector.y = GetValue(1)->Downcast<RN::Number>()->GetFloatValue();
		vector.z = GetValue(2)->Downcast<RN::Number>()->GetFloatValue();
		
		_observable->SetValue(RN::Value::WithQuaternion(RN::Quaternion(vector)));
	}
	
	void QuaternionPropertyView::ValueDidChange(RN::Object *newValue)
	{
		RN::Value *value = newValue->Downcast<RN::Value>();
		RN::Quaternion rotation = value->GetValue<RN::Quaternion>();
		RN::Vector3 vector = rotation.GetEulerAngle();
		
		SetValue(RN::Number::WithFloat(vector.x), 0);
		SetValue(RN::Number::WithFloat(vector.y), 1);
		SetValue(RN::Number::WithFloat(vector.z), 2);
	}
	
	// -----------------------
	// MARK: -
	// MARK: ColorPropertyView
	// -----------------------
	
	ColorPropertyView::ColorPropertyView(RN::ObservableProperty *observable, RN::String *title) :
		ComponentPropertyView(observable, title, 3)
	{
		SetTitle(RNCSTR("R"), 0);
		SetTitle(RNCSTR("G"), 1);
		SetTitle(RNCSTR("B"), 2);
		
		ValueDidChange(_observable->GetValue());
	}
	
	void ColorPropertyView::ValueAtIndexChanged(size_t index)
	{
		RN::Color color;
		
		color.r = GetValue(0)->Downcast<RN::Number>()->GetFloatValue();
		color.g = GetValue(1)->Downcast<RN::Number>()->GetFloatValue();
		color.b = GetValue(2)->Downcast<RN::Number>()->GetFloatValue();
		color.a = 1.0f;
		
		_observable->SetValue(RN::Value::WithColor(color));
	}
	
	void ColorPropertyView::ValueDidChange(RN::Object *newValue)
	{
		RN::Value *value = newValue->Downcast<RN::Value>();
		RN::Color color  = value->GetValue<RN::Color>();
		
		SetValue(RN::Number::WithFloat(color.r), 0);
		SetValue(RN::Number::WithFloat(color.g), 1);
		SetValue(RN::Number::WithFloat(color.b), 2);
	}
}
