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
		enum class Mode
		{
			Add,
			Substract
		};
		
		SculptTool(class Viewport *viewport);
		~SculptTool();
		
		void UpdateEditMode(float delta) override;
		
		void SetTarget(RN::Sculptable *target);
		void SetMode(Mode mode);
		
		void UseTool();
		
		void SetRadius(float radius);
		float GetRadius() const { return _radius; }
		
	private:
		RN::Sculptable *_target;
		Viewport *_viewport;
		bool _hasValidPosition;
		Mode _mode;
		
		RN::Observable<float, SculptTool> _radius;
		
		RNDeclareMeta(SculptTool)
	};
}

#endif /* defined(__Downpour__DPSculptTool__) */
