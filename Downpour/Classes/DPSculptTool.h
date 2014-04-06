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
#include "DPWorkspace.h"

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
		
	private:
		RN::Sculptable *_target;
		Viewport *_viewport;
		bool _checkLastPosition;
		RN::Vector3 _lastPosition;
		Mode _mode;
		
		Workspace::Tool _previousTool;
		
		RNDeclareMeta(SculptTool)
	};
}

#endif /* defined(__Downpour__DPSculptTool__) */
