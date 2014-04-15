//
//  DPSculptTool.h
//  Downpour
//
//  Created by Nils Daumann on 05.04.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
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
