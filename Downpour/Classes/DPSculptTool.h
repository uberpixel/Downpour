//
//  DPSculptTool.h
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

#ifndef __Downpour__DPSculptTool__
#define __Downpour__DPSculptTool__

#include <Rayne/Rayne.h>

namespace DP
{
	class SculptTool : public RN::Entity
	{
	public:
		enum Mode
		{
			Add,
			Substract
		};
		
		enum Shape
		{
			Sphere = 0,
			Cube = 1
		};
		
		SculptTool(class Viewport *viewport);
		~SculptTool();
		
		void UpdateEditMode(float delta) override;
		
		void SetTarget(RN::Sculptable *target);
		void SetMode(Mode mode);
		void SetShape(Shape shape);
		
		void UseTool();
		
		void SetRadius(float radius);
		float GetRadius() const { return _radius; }
		
		void SetSize(const RN::Vector3 &size);
		RN::Vector3 GetSize() const { return _size; }
		
	private:
		RN::Sculptable *_target;
		Viewport *_viewport;
		bool _hasValidPosition;
		Mode _mode;
		Shape _shape;
		
		RN::Material::ShaderUniform *_positionUniform[2];
		RN::Material::ShaderUniform *_sizeUniform[2];
		
		RN::Model *_models[2];
		
		RN::Observable<float, SculptTool> _radius;
		RN::Observable<RN::Vector3, SculptTool> _size;
		
		RNDeclareMeta(SculptTool)
	};
}

#endif /* defined(__Downpour__DPSculptTool__) */
