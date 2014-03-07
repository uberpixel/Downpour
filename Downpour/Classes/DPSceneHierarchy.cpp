//
//  DPSceneHierarchy.cpp
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

#include "DPSceneHierarchy.h"
#include "DPWorkspace.h"
#include "DPColorScheme.h"

namespace DP
{
	SceneHierarchy::SceneHierarchy()
	{
		_data = new RN::Array();
		
		{
			RN::Array *sceneGraph = RN::World::GetActiveWorld()->GetSceneNodes();
			sceneGraph->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &flags) {
				
				if(!node->GetParent())
					_data->AddObject(node);
				
			});
		}
			
		_tree = new RN::UI::OutlineView();
		_tree->SetAutoresizingMask(RN::UI::View::AutoresizingFlexibleHeight | RN::UI::View::AutoresizingFlexibleWidth);
		_tree->SetDataSource(this);
		_tree->SetDelegate(this);
		_tree->SetAllowsMultipleSelection(true);
		_tree->ReloadData();
		
		AddSubview(_tree);
		SetBackgroundColor(ColorScheme::GetColor(ColorScheme::Type::Background));
	}
	
	SceneHierarchy::~SceneHierarchy()
	{
		_data->Release();
		_tree->Release();
	}
	
	// -----------------------
	// MARK: -
	// MARK: RN::UI::OutlineViewDataSource
	// -----------------------
	
	bool SceneHierarchy::OutlineViewItemIsExpandable(RN::UI::OutlineView *outlineView, void *item)
	{
		RN::SceneNode *node = static_cast<RN::SceneNode *>(item);
		return node->HasChildren();
	}
	
	size_t SceneHierarchy::OutlineViewGetNumberOfChildrenForItem(RN::UI::OutlineView *outlineView, void *item)
	{
		if(!item)
			return _data->GetCount();
		
		RN::SceneNode *node = static_cast<RN::SceneNode *>(item);
		if(!node->HasChildren())
			return 0;
		
		const RN::Array *children = node->GetChildren();
		return children->GetCount();
	}
	
	void *SceneHierarchy::OutlineViewGetChildOfItem(RN::UI::OutlineView *outlineView, void *item, size_t child)
	{
		if(!item)
			return _data->GetObjectAtIndex(child);
		
		RN::SceneNode *node = static_cast<RN::SceneNode *>(item);
		
		const RN::Array *children = node->GetChildren();
		return children->GetObjectAtIndex(child);
	}
	
	RN::UI::OutlineViewCell *SceneHierarchy::OutlineViewGetCellForItem(RN::UI::OutlineView *outlineView, void *item)
	{
		RN::String *identifier = RNCSTR("Cell");
		OutlineViewCell *cell = static_cast<OutlineViewCell *>(outlineView->DequeCellWithIdentifier(identifier));
		
		if(!cell)
		{
			cell = new OutlineViewCell(identifier);
			cell->Autorelease();
		}
		
		RN::SceneNode *node = static_cast<RN::SceneNode *>(item);
		
		std::string name = node->GetDebugName();
		if(name.empty())
			name = node->Class()->Name();
		
		cell->GetTextLabel()->SetText(RNSTR(name.c_str()));
		
		return cell;
	}
	
	
	// -----------------------
	// MARK: -
	// MARK: RN::UI::OutlineViewDelegate
	// -----------------------
	
	void SceneHierarchy::OutlineViewSelectionDidChange(RN::UI::OutlineView *outlineView)
	{
		RN::IndexSet *selection = outlineView->GetSelection();
		
		if(selection->GetCount() > 0)
		{
			RN::Array *items = new RN::Array(selection->GetCount());
			
			size_t count = selection->GetCount();
			for(size_t i = 0; i < count; i ++)
			{
				RN::SceneNode *node = static_cast<RN::SceneNode *>(outlineView->GetItemForRow(selection->GetIndex(i)));
				items->AddObject(node);
			}
			
			Workspace::GetSharedInstance()->SetSelection(items);
			items->Release();
		}
		else
		{
			Workspace::GetSharedInstance()->SetSelection(nullptr);
		}
	}
}
