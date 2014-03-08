//
//  DPWorkspace.h
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

#ifndef __DPWORKSPACE_H__
#define __DPWORKSPACE_H__

#include <Rayne/Rayne.h>
#include "DPWidgetContainer.h"
#include "DPFileTree.h"
#include "DPViewport.h"
#include "DPInspectorView.h"
#include "DPSceneHierarchy.h"
#include "DPWorldAttachment.h"
#include "DPSavedState.h"

#define kDPWorkspaceSelectionChanged RNCSTR("kDPWorkspaceSelectionChanged")

namespace DP
{
	class Workspace : public RN::UI::Widget, public RN::INonConstructingSingleton<Workspace>
	{
	public:
		Workspace(RN::Module *module);
		~Workspace() override;
		
		std::string GetResourcePath() const { return RN::PathManager::Join(_module->GetPath(), "Resources"); }
		RN::Array *GetSelection() const { return _selection; }
		
		SavedState *GetSavedState() const { return _state; }
		
		void SetSelection(RN::Array *selection);
		void SetSelection(RN::SceneNode *selection);
		void SetSelection(std::nullptr_t null);
		
	private:
		void UpdateSize();
		
		SavedState *_state;
		WorldAttachment *_worldAttachment;
		
		WidgetContainer<FileTree> *_fileTree;
		WidgetContainer<Viewport> *_viewport;
		WidgetContainer<InspectorViewContainer> *_inspectors;
		WidgetContainer<SceneHierarchy> *_hierarchy;
		
		RN::Array *_selection;
		
		RN::Module *_module;
		
		RNDeclareSingleton(Workspace)
	};
}

#endif /* __DPWORKSPACE_H__ */
