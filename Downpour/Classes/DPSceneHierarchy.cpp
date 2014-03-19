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
#include "DPWorldAttachment.h"
#include "DPColorScheme.h"

#include "DPEditorIcon.h"

namespace DP
{
	SceneHierarchy::SceneHierarchy() :
		_suppressSelectionNotification(false)
	{
		{
			RN::Array *sceneGraph = RN::World::GetActiveWorld()->GetSceneNodes();
			sceneGraph->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &flags) {
				
				if(!(node->GetFlags() & RN::SceneNode::Flags::HideInEditor))
				{
					if(!node->GetParent())
						_data.emplace_back(new SceneNodeProxy(node));
					
					if(EditorIcon::SupportsSceneNodeClass(node))
						EditorIcon::WithSceneNode(node);
				}
			});
			
			// GetSceneNodes() returns the scene nodes in an unsorted, "random" way
			// The hierarchy however should display the nodes in deterministic order, which is achieved by
			// sorting them by their UID
			// This is not needed for child nodes since their order is defined
			
			std::sort(_data.begin(), _data.end(), [](const SceneNodeProxy *a, const SceneNodeProxy *b) {
				return (a->node->GetLID() < b->node->GetLID());
			});
		}
			
		_tree = new RN::UI::OutlineView();
		_tree->SetAutoresizingMask(RN::UI::View::AutoresizingFlexibleHeight | RN::UI::View::AutoresizingFlexibleWidth);
		_tree->SetDataSource(this);
		_tree->SetDelegate(this);
		_tree->SetAllowsMultipleSelection(true);
		_tree->ReloadData();
		
		AddSubview(_tree);
		
		RN::MessageCenter::GetSharedInstance()->AddObserver(kDPWorkspaceSelectionChanged, [this](RN::Message *message) {
			
			if(_suppressSelectionNotification)
				return;
			
			RN::IndexSet *selection = new RN::IndexSet();
			RN::Array *objects = static_cast<RN::Array *>(message->GetObject());
			
			bool hasVisibleRow = false;
			
			if(objects)
			{
				RN::Range range = _tree->GetVisibleRange();
				
				objects->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop) {
					
					size_t row = _tree->GetRowForItem(FindProxyForNode(node));
					if(row == kRNNotFound)
						return;
					
					selection->AddIndex(row);
					
					if(range.origin <= row && row <= range.GetEnd())
						hasVisibleRow = true;
				});
			}
			
			_suppressSelectionNotification = true;
			_tree->SetSelection(selection);
			_suppressSelectionNotification = false;
			
			// Scroll the first selection into the visible area
			if(!hasVisibleRow && selection->GetCount() > 0)
				_tree->ScrollToRow(selection->GetFirstIndex(), RN::UI::TableView::ScrollPosition::Top);
			
			selection->Release();
			
		}, this);
		
		RN::MessageCenter::GetSharedInstance()->AddObserver(kDPWorldAttachmentDidAddSceneNode, &SceneHierarchy::DidAddSceneNode, this, this);
		RN::MessageCenter::GetSharedInstance()->AddObserver(kDPWorldAttachmentWillRemoveSceneNode, &SceneHierarchy::WillRemoveSceneNode, this, this);
	}
	
	SceneHierarchy::~SceneHierarchy()
	{
		for(SceneNodeProxy *proxy : _data)
		{
			delete proxy;
		}
		
		_tree->Release();
		
		RN::MessageCenter::GetSharedInstance()->RemoveObserver(this);
	}
	
	// -----------------------
	// MARK: -
	// MARK: Scene graph
	// -----------------------
	
	SceneNodeProxy *SceneHierarchy::FindProxyForNode(RN::SceneNode *node)
	{
		for(SceneNodeProxy *proxy : _data)
		{
			SceneNodeProxy *temp = proxy->FindProxyForNode(node);
			if(temp)
				return temp;
		}
		
		return nullptr;
	}
	
	void SceneHierarchy::DidAddSceneNode(RN::Message *message)
	{
		RN::SceneNode *node = static_cast<RN::SceneNode *>(message->GetObject());
		
		if(node->GetFlags() & RN::SceneNode::Flags::HideInEditor)
			return;
		
		if(EditorIcon::SupportsSceneNodeClass(node))
			EditorIcon::WithSceneNode(node);
		
		RN::SceneNode *parent = node->GetParent();
		if(!parent)
		{
			_data.emplace_back(new SceneNodeProxy(node));
			_tree->ReloadItem(nullptr, false);
		}
		else
		{
			SceneNodeProxy *proxy = FindProxyForNode(parent);
			if(proxy && !proxy->ContainsProyForNode(node))
			{
				proxy->children.emplace_back(new SceneNodeProxy(node));
				_tree->ReloadItem(proxy, true);
			}
		}
	}
	
	void SceneHierarchy::WillRemoveSceneNode(RN::Message *message)
	{
		RN::SceneNode *node = static_cast<RN::SceneNode *>(message->GetObject());
		
		if(node->GetFlags() & RN::SceneNode::Flags::HideInEditor)
			return;
		
		EditorIcon *icon = EditorIcon::GetIconForSceneNode(node);
		if(icon)
			icon->Detach();
		
		RN::SceneNode *parent = node->GetParent();
		if(parent)
		{
			SceneNodeProxy *proxy = FindProxyForNode(parent);
			if(proxy)
			{
				proxy->RemoveProxyForNode(node);
				_tree->ReloadItem(proxy, true);
			}
		}
		else
		{
			for(auto i = _data.begin(); i != _data.end(); i ++)
			{
				SceneNodeProxy *proxy = *i;
				
				if(proxy->node == node)
				{
					delete proxy;
					_data.erase(i);
					
					break;
				}
			}
			
			_tree->ReloadItem(nullptr, false);
		}
	}
	
	// -----------------------
	// MARK: -
	// MARK: RN::UI::OutlineViewDataSource
	// -----------------------
	
	bool SceneHierarchy::OutlineViewItemIsExpandable(RN::UI::OutlineView *outlineView, void *item)
	{
		SceneNodeProxy *proxy = static_cast<SceneNodeProxy *>(item);
		return proxy->IsExpandable();
	}
	
	size_t SceneHierarchy::OutlineViewGetNumberOfChildrenForItem(RN::UI::OutlineView *outlineView, void *item)
	{
		if(!item)
			return _data.size();
		
		SceneNodeProxy *proxy = static_cast<SceneNodeProxy *>(item);
		return proxy->children.size();
	}
	
	void *SceneHierarchy::OutlineViewGetChildOfItem(RN::UI::OutlineView *outlineView, void *item, size_t child)
	{
		if(!item)
			return _data[child];
		
		SceneNodeProxy *proxy = static_cast<SceneNodeProxy *>(item);
		return proxy->children[child];
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
		
		RN::SceneNode *node = static_cast<SceneNodeProxy *>(item)->node;
		
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
		if(_suppressSelectionNotification)
			return;
		
		RN::IndexSet *selection = outlineView->GetSelection();
		
		if(selection->GetCount() > 0)
		{
			RN::Array *items = new RN::Array(selection->GetCount());
			
			size_t count = selection->GetCount();
			for(size_t i = 0; i < count; i ++)
			{
				RN::SceneNode *node = static_cast<SceneNodeProxy *>(outlineView->GetItemForRow(selection->GetIndex(i)))->node;
				items->AddObject(node);
			}
			
			_suppressSelectionNotification = true;
			Workspace::GetSharedInstance()->SetSelection(items);
			_suppressSelectionNotification = false;
			
			items->Release();
		}
		else
		{
			_suppressSelectionNotification = true;
			Workspace::GetSharedInstance()->SetSelection(nullptr);
			_suppressSelectionNotification = false;
		}
	}
}
