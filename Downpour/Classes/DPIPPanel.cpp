//
//  DPIPPanel.cpp
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

#include "DPIPPanel.h"
#include "DPColorScheme.h"

namespace DP
{
	IPPanel::IPPanel(std::function<void (std::string)> &&callback) :
		RN::UI::Widget(RN::UI::Widget::Style::Titled | RN::UI::Widget::Style::Closable, RN::Rect(0.0f, 0.0f, 300.0f, 110.0f))
	{
		SetTitle(RNCSTR("Connect to Server"));
		
		_messageLabel = new RN::UI::Label();
		_messageLabel->SetTextColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_Text));
		_messageLabel->SetFrame(RN::Rect(0.0f, 0.0f, 300.0f, 340.0f).Inset(10.0f, 10.0f));
		_messageLabel->SetText(RNCSTR("IP Address of host:"));
		
		_ipField = RN::UI::TextField::WithType(RN::UI::TextField::Type::Bezel);
		_ipField->SetFrame(RN::Rect(0.0f, 30.0f, 300.0f, 20.0f).Inset(10.0f, 0.0f));
		_ipField->SetText(RN::Settings::GetSharedInstance()->GetObjectForKey<RN::String>(RNCSTR("DPHostIP")));
		
		_connectButton = new RN::UI::Button(RN::UI::Button::Type::Bezel);
		_connectButton->SetTitleColorForState(ColorScheme::GetColor(ColorScheme::Type::FileTree_Text), RN::UI::Control::State::Normal);
		_connectButton->SetTitleForState(RNCSTR("Connect"), RN::UI::Control::State::Normal);
		_connectButton->SetFrame(RN::Rect(110.0f, 70.0f, 80.0f, 30.0f));
		_connectButton->AddListener(RN::UI::Control::EventType::MouseUpInside, [this, callback](RN::UI::Control *, RN::UI::Control::EventType) {
			
			RN::String *ip = _ipField->GetText();
			RN::Settings::GetSharedInstance()->SetObjectForKey(ip, RNCSTR("DPHostIP"));
			
			callback(std::string(ip->GetUTF8String()));
			Close();
			
		}, nullptr);
		
		GetContentView()->AddSubview(_messageLabel->Autorelease());
		GetContentView()->AddSubview(_connectButton->Autorelease());
		GetContentView()->AddSubview(_ipField);
		
		Center();
	}
}
