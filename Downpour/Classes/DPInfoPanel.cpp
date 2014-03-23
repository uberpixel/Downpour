//
//  DPInfoPanel.cpp
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

#include "DPInfoPanel.h"
#include "DPColorScheme.h"

namespace DP
{
	InfoPanel::InfoPanel() :
		RN::UI::Widget(RN::UI::Widget::Style::Titled | RN::UI::Widget::Style::Closable, RN::Rect(0.0f, 0.0f, 500.0f, 380.0f)),
		_message(nullptr)
	{
		SetTitle(RNCSTR("Info"));
		Center();
		
		_messageLabel = new RN::UI::Label();
		_messageLabel->SetNumberOfLines(0);
		_messageLabel->SetTextColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_Text));
		_messageLabel->SetFrame(RN::Rect(0.0f, 0.0f, 500.0f, 340.0f).Inset(10.0f, 10.0f));
		_messageLabel->SetLineBreak(RN::UI::LineBreakMode::WordWrapping);
		
		_okButton = new RN::UI::Button(RN::UI::Button::Type::Bezel);
		_okButton->SetTitleColorForState(ColorScheme::GetUIColor(ColorScheme::Type::FileTree_Text), RN::UI::Control::State::Normal);
		_okButton->SetTitleForState(RNCSTR("Ok"), RN::UI::Control::State::Normal);
		_okButton->SetFrame(RN::Rect(210.0f, 340.0f, 80.0f, 30.0f));
		_okButton->AddListener(RN::UI::Control::EventType::MouseUpInside, std::bind(&InfoPanel::Close, this), nullptr);
		
		GetContentView()->AddSubview(_messageLabel);
		GetContentView()->AddSubview(_okButton);
	}
	
	InfoPanel::~InfoPanel()
	{
		_okButton->Release();
		_messageLabel->Release();
		
		RN::SafeRelease(_message);
	}
	
	void InfoPanel::SetMessage(RN::String *message)
	{
		_messageLabel->SetText(message);
	}
	
	
	
	ExceptionPanel::ExceptionPanel(RN::Exception &e)
	{
		RN::String *message = RNSTR("Caught exception '%s'\nReason: %s\n\nBacktrace:\n", e.GetStringifiedType(), e.GetReason().c_str());
	
		const std::vector<std::pair<uintptr_t, std::string>> &callstack = e.GetCallStack();
		for(auto &pair : callstack)
		{
			message->Append("0x%x %s\n", pair.first, pair.second.c_str());
		}
	
		SetMessage(message);
	}
}
