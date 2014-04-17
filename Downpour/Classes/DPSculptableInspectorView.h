//
//  DPSculptableInspectorView.h
//  Downpour
//
//  Created by Nils Daumann on 06.04.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
//

#ifndef __Downpour__DPSculptableInspectorView__
#define __Downpour__DPSculptableInspectorView__

#include <Rayne/Rayne.h>
#include "DPInspectorView.h"
#include "DPSculptTool.h"
#include "DPWorkspace.h"

namespace DP
{
	class SculptableInspectorView : public InspectorView
	{
	public:
		SculptableInspectorView();
		~SculptableInspectorView();
		
		void Initialize(RN::Object *object, RN::MetaClass *meta, RN::String *title) override;
		
		static void InitialWakeUp(RN::MetaClass *meta);
		
	private:
		Workspace::Tool _previousTool;
		RNDeclareMeta(SculptableInspectorView)
	};
}

#endif /* defined(__Downpour__DPSculptableInspectorView__) */
