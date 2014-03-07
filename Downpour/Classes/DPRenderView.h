//
//  DPRenderView.h
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

#ifndef __DPRENDERVIEW_H__
#define __DPRENDERVIEW_H__

#include <Rayne/Rayne.h>

namespace DP
{
	class RenderView : public RN::UI::View
	{
	public:
		RNAPI RenderView();
		RNAPI ~RenderView() override;
		
		RNAPI void SetTexture(RN::Texture *texture);
		RNAPI void SetScaleMode(RN::UI::ScaleMode mode);
		RNAPI void SetFrame(const RN::Rect& frame) override;
		
		RNAPI RN::UI::ScaleMode GetScaleMode() const { return _scaleMode; }
		
		RNAPI RN::Vector2 GetSizeThatFits() override;
		
	protected:
		RNAPI void Update() override;
		RNAPI void Draw(RN::Renderer *renderer) override;
		
	private:
		void Initialize();
		
		bool _isDirty;
		
		RN::UI::ScaleMode _scaleMode;
		RN::Texture *_texture;
		
		RN::Material *_material;
		RN::Mesh  *_mesh;
		
		RNDeclareMeta(RenderView, View)
	};
}

#endif /* __DPRENDERVIEW_H__ */
