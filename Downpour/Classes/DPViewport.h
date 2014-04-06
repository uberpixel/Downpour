//
//  DPViewport.h
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

#ifndef __DPVIEWPORT_H__
#define __DPVIEWPORT_H__

#include <Rayne/Rayne.h>
#include "DPRenderView.h"
#include "DPDragNDropTarget.h"

namespace DP
{
	class Viewport : public DragNDropTarget
	{
	public:
		Viewport();
		~Viewport() override;
		
		void SetFrame(const RN::Rect &frame) override;
		void Update() override;
		
		bool AcceptsDropOfObject(RN::Object *object) final;
		void HandleDropOfObject(RN::Object *object, const RN::Vector2 &position) final;
		
		void UpdateSourceCamera(RN::Camera *source);
		
		RN::Camera *GetCamera() const { return _camera; }
		RN::Camera *GetEditorCamera() const { return _editorCamera; }
		
		RN::Vector2 ConvertPointToViewport(const RN::Vector2 &point);
		RN::Vector3 GetDirectionForMouse(const RN::Vector2 &point);
		RN::Vector3 GetPositionForMouse(const RN::Vector2 &tpoint, float dist);
		
	private:
		RN::Vector3 GetDirectionForPoint(const RN::Vector2 &point);
		RN::Vector3 GetPositionForPoint(const RN::Vector2 &tpoint, float dist);
		bool CanBecomeFirstResponder() override;
		
		void MouseDown(RN::Event *event) override;
		void MouseDragged(RN::Event *event) override;
		void MouseUp(RN::Event *event) override;
		void MouseMoved(RN::Event *event) override;
		
		RN::Camera *_camera;
		RN::Camera *_editorCamera;
		RN::Camera *_postProcessCamera;
		
		float _resolutionFactor;
		
		RN::Camera *_sourceCamera;
		
		RenderView *_renderView;
	};
}

#endif /* __DPVIEWPORT_H__ */
