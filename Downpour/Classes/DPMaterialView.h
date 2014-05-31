//
//  DPMaterialView.h
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

#ifndef __DPMATERIALVIEW_H__
#define __DPMATERIALVIEW_H__

#include <Rayne/Rayne.h>

namespace DP
{
	class MaterialView : public RN::UI::View
	{
	public:
		MaterialView(RN::Material *material);
		~MaterialView();
		
		void LayoutSubviews() override;
		RN::Vector2 GetSizeThatFits() override;
		
	private:
		void InsertTexture(RN::Texture *texture);
		void InsertBooleanWithTitle(RN::String *title, std::function<void (bool)> &&setter, bool state);
		void InsertFloatWithTitle(RN::String *title, std::function<void (float)> &&setter, float value);
		void InsertColorWithTitle(RN::String *title, std::function<void (const RN::Color &)> &&setter, const RN::Color &color);
		void InsertEnumWithTitle(RN::String *title, std::function<void (size_t)> &&setter, size_t value, RN::UI::Menu *menu);
		void InsertViewWithTitle(RN::String *title, RN::UI::View *view);
		
		RN::Material *_material;
		
		RN::Array *_textureViews;
		RN::Array *_views;
		
		RNDeclareMeta(MaterialView)
	};
	
	class MaterialWidget : public RN::UI::Widget, public RN::UI::ScrollViewDelegate
	{
	public:
		MaterialWidget(RN::Material *material);
		
		RNAPI void ScrollViewDidChangeScrollerInset(RN::UI::ScrollView *scrollView, const RN::UI::EdgeInsets &insets) override;
		
	private:
		MaterialView *_materialView;
		RNDeclareMeta(MaterialWidget)
	};
}

#endif /* __DPMATERIALVIEW_H__ */
