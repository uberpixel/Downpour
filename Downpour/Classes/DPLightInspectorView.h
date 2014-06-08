//
//  DPLightInspectorView.h
//  Downpour
//
//  Created by Nils Daumann on 08.06.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
//

#ifndef __Downpour__DPLightInspectorView__
#define __Downpour__DPLightInspectorView__

#include <Rayne/Rayne.h>
#include "DPInspectorView.h"
#include "DPSculptTool.h"
#include "DPWorkspace.h"

namespace DP
{
	class LightInspectorView : public InspectorView
	{
	public:
		LightInspectorView();
		~LightInspectorView();
		
		void Initialize(RN::Object *object, RN::MetaClass *meta, RN::String *title) override;
		
		static void InitialWakeUp(RN::MetaClass *meta);
		
	private:
		RNDeclareMeta(LightInspectorView)
	};
}

#endif /* defined(__Downpour__DPLightInspectorView__) */
