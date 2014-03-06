//
//  DPColorScheme.cpp
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

#include "DPColorScheme.h"

namespace DP
{
	namespace ColorScheme
	{
		static std::unordered_map<uint32, RN::Color> __colorTable;
		
		void LoadColor(Type type, const char *name)
		{
			RN::String *key = RNSTR("downpour.colors.%s", name);
			RN::UI::Color *color = RN::UI::Style::GetSharedInstance()->GetColorWithKeyPath(key);
			
			__colorTable[static_cast<uint32>(type)] = color->GetRNColor();
		}
		
		const RN::Color &GetColor(Type type)
		{
			static std::once_flag token;
			std::call_once(token, [] {
				
				RN::AutoreleasePool pool;
				
				LoadColor(Type::Background, "background");
				LoadColor(Type::FileTree_Selection, "filetree_selection");
				LoadColor(Type::FileTree_Text, "filetree_text");
				LoadColor(Type::FileTree_TextSelection, "filetree_text_selection");
				
			});
			
			return __colorTable[static_cast<uint32>(type)];
		}
	}
}
