//
//  DPColorScheme.h
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

#ifndef __DPCOLORSCHEME_H__
#define __DPCOLORSCHEME_H__

#include <Rayne/Rayne.h>

namespace DP
{
	namespace ColorScheme
	{
		enum class Type : uint32
		{
			Background,
			Background_Light,
			FileTree_Selection,
			FileTree_Text,
			FileTree_TextSelection
		};
		
		RN::UI::Color *GetColor(Type type);
	}
	
	class OutlineViewCell : public RN::UI::OutlineViewCell
	{
	public:
		OutlineViewCell(RN::String *identifier) :
			RN::UI::OutlineViewCell(identifier)
		{
			SetSelected(IsSelected());
		}
		
		void SetSelected(bool selected) override
		{
			RN::UI::OutlineViewCell::SetSelected(selected);
			
			if(selected)
			{
				SetBackgroundColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_Selection));
				GetTextLabel()->SetTextColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_TextSelection));
			}
			else
			{
				SetBackgroundColor(RN::UI::Color::ClearColor());
				GetTextLabel()->SetTextColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_Text));
			}
		}
	};
}

#endif /* __DPCOLORSCHEME_H__ */
