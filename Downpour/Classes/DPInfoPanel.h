//
//  DPInfoPanel.h
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

#ifndef __DPINFOPANEL_H__
#define __DPINFOPANEL_H__

#include <Rayne/Rayne.h>

namespace DP
{
	class InfoPanel : public RN::UI::Widget
	{
	public:
		InfoPanel();
		~InfoPanel();
		
		void SetMessage(RN::String *message);
		
		static InfoPanel *WithMessage(RN::String *message);
		
	private:
		RN::String *_message;
		
		RN::UI::Label *_messageLabel;
		RN::UI::Button *_okButton;
		
		RNDeclareMeta(InfoPanel)
	};
	
	class ExceptionPanel : public InfoPanel
	{
	public:
		ExceptionPanel(RN::Exception &e);
	};
}

#endif /* __DPINFOPANEL_H__ */
