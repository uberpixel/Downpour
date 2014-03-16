//
//  DPWidgetContainer.h
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

#ifndef __DPWIDGETCONTAINER_H__
#define __DPWIDGETCONTAINER_H__

#include <Rayne/Rayne.h>
#include "DPColorScheme.h"

#define kDPWidgetContainerTabWidth  90.0f
#define kDPWidgetContainerTabHeight 25.0f

namespace DP
{
	template<class T>
	class WidgetContainer : public RN::UI::View
	{
	public:
		template<class ...Args>
		WidgetContainer(RN::String *title, Args &&... args)
		{
			_titleLabel = new RN::UI::Label();
			_titleLabel->SetText(title);
			_titleLabel->SetAlignment(RN::UI::TextAlignment::Center);
			_titleLabel->SetTextColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_Text));
			_titleLabel->SetFont(RN::UI::Style::GetSharedInstance()->GetFont(RN::UI::Style::FontStyle::DefaultFontBold));
			_titleLabel->SetFrame(RN::Rect(5.0f, 2.0f, kDPWidgetContainerTabWidth - 10.0f, kDPWidgetContainerTabHeight - 2.0f));
			
			_tab = new RN::UI::View();
			_tab->SetFrame(RN::Rect(20.0f, 5.0f, kDPWidgetContainerTabWidth, kDPWidgetContainerTabHeight));
			_tab->SetBackgroundColor(ColorScheme::GetColor(ColorScheme::Type::Background));
			_tab->AddSubview(_titleLabel);
			
			_content = new T(std::forward<Args>(args)...);
			_content->SetAutoresizingMask(RN::UI::View::AutoresizingFlexibleHeight | RN::UI::View::AutoresizingFlexibleWidth);
			_content->SetBackgroundColor(ColorScheme::GetColor(ColorScheme::Type::Background));
			
			_container = new RN::UI::View();
			_container->AddSubview(_content);
			
			AddSubview(_tab);
			AddSubview(_container);
			
			SetBackgroundColor(ColorScheme::GetColor(ColorScheme::Type::Background_Light));
		}
		
		~WidgetContainer()
		{
			_tab->Release();
			_titleLabel->Release();
			
			_container->Release();
			_content->Release();
		}
		
		void LayoutSubviews() override
		{
			RN::UI::View::LayoutSubviews();
			
			RN::Rect frame = GetBounds();
			_container->SetFrame(RN::Rect(0.0f, kDPWidgetContainerTabHeight, frame.width, frame.height - kDPWidgetContainerTabHeight));
		}
		
		T *GetContent() const { return _content; }
	
	private:
		RN::UI::Label *_titleLabel;
		RN::UI::View *_tab;
		
		RN::UI::View *_container;
		
		T *_content;
	};
}

#endif /* __DPWIDGETCONTAINER_H__ */
