//
//  DPNodeClassPicker.cpp
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

#include "DPNodeClassPicker.h"
#include "DPColorScheme.h"

namespace DP
{
	NodeClassPicker::NodeClassPicker() :
		_root(new ClassProxy(RN::SceneNode::MetaClass()))
	{
		_tree = new DraggableOutlineView();
		_tree->SetAutoresizingMask(RN::UI::View::AutoresizingMask::FlexibleHeight | RN::UI::View::AutoresizingMask::FlexibleWidth);
		_tree->SetDataSource(this);
		_tree->SetDelegate(this);
		_tree->ReloadData();
		_tree->ExpandItem(nullptr, true);
		
		AddSubview(_tree->Autorelease());
	}
	
	NodeClassPicker::~NodeClassPicker()
	{
		delete _root;
	}
	
	// -----------------------
	// MARK: -
	// MARK: Drag and Drop
	// -----------------------
	
	bool NodeClassPicker::CanDragItemAtRow(DraggableOutlineView *outlineView, void *item, size_t row)
	{
		return true;
	}
	
	RN::Object *NodeClassPicker::GetObjectForDraggedItem(DraggableOutlineView *outlineView, void *item)
	{
		ClassProxy *proxy = static_cast<ClassProxy *>(item);
		RN::Value *value = new RN::Value(proxy->meta);
		
		return value->Autorelease();
	}
	
	// -----------------------
	// MARK: -
	// MARK: RN::UI::OutlineViewDataSource
	// -----------------------
	
	bool NodeClassPicker::OutlineViewItemIsExpandable(RN::UI::OutlineView *outlineView, void *item)
	{
		ClassProxy *proxy = static_cast<ClassProxy *>(item);
		return !proxy->children.empty();
	}
	
	size_t NodeClassPicker::OutlineViewGetNumberOfChildrenForItem(RN::UI::OutlineView *outlineView, void *item)
	{
		if(!item)
			return 1;
		
		ClassProxy *proxy = static_cast<ClassProxy *>(item);
		return proxy->children.size();
	}
	
	void *NodeClassPicker::OutlineViewGetChildOfItem(RN::UI::OutlineView *outlineView, void *item, size_t child)
	{
		if(!item)
			return _root;
		
		ClassProxy *proxy = static_cast<ClassProxy *>(item);
		return proxy->children[child];
	}
	
	RN::UI::OutlineViewCell *NodeClassPicker::OutlineViewGetCellForItem(RN::UI::OutlineView *outlineView, void *item)
	{
		RN::String *identifier = RNCSTR("Cell");
		DraggableOutlineViewCell *cell = static_cast<DraggableOutlineViewCell *>(outlineView->DequeCellWithIdentifier(identifier));
		
		if(!cell)
		{
			cell = new DraggableOutlineViewCell(_tree, identifier);
			cell->Autorelease();
		}
		
		ClassProxy *proxy = static_cast<ClassProxy *>(item);
		
		cell->GetTextLabel()->SetText(proxy->name);
		
		return cell;
	}
}