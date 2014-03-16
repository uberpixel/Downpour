//
//  DPGizmo.h
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

#ifndef __DPGIZMO_H__
#define __DPGIZMO_H__

#include <Rayne/Rayne.h>

namespace DP
{
	class Gizmo : public RN::Entity
	{
	public:
		enum class Mode
		{
			Translate = 0,
			Scale = 1,
			Rotate = 2
		};
		
		enum class Space
		{
			Local = 0,
			Global = 1
		};
		
		Gizmo(RN::Camera *camera);
		~Gizmo();
		
		void SetSelection(RN::Array *selection);
		void SetMode(Mode mode);
		void SetSpace(Space space);
		
		Mode GetMode() const { return _mode; }
		Space GetSpace() const { return _space; }
		
		void UpdateEditMode(float delta) override;
		bool IsActive() const { return _active; }
		
		void SetHighlight(uint32 selection, float factor=2.0f);
		
		void BeginMove(uint32 selection, const RN::Vector2 &mousePos);
		void ContinueMove(const RN::Vector2 &mousePos);
		void EndMove();
		
	private:
		RN::Vector3 CameraToWorld(const RN::Vector3 &dir);
		RN::Vector3 GetMouseMovement(const RN::Plane &plane, RN::Vector2 from, RN::Vector2 to);
		RN::Vector3 GetMousePosition(const RN::Plane &plane, RN::Vector2 mouse);
		
		void DoTranslation(const RN::Vector2 &mousePos);
		void DoScale(RN::Vector2 mousePos);
		void DoRotation(RN::Vector2 mousePos);
		
		RN::Camera *_camera;
		RN::Array  *_selection;
		
		RN::Model *_modelTranslation;
		RN::Model *_modelScaling;
		RN::Model *_modelRotation;
		float _scaleFactor;
		
		Mode _mode;
		Space _space;
		
		bool _active;
		uint32 _selectedMesh;
		RN::Color _highlightOldColor;
		RN::Vector2 _previousMouse;
		
		RNDeclareMeta(Gizmo)
	};
}

#endif /* __DPGIZMO_H__ */
