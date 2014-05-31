//
//  DPFileTree.cpp
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

#include "DPFileTree.h"
#include "DPColorScheme.h"
#include "DPWorkspace.h"
#include "DPDragNDropTarget.h"

const char *kDPFileTreeChangesAssociationKey = "kDPFileTreeChangesAssociationKey";

namespace DP
{
	// -----------------------
	// MARK: -
	// MARK: FileTree
	// -----------------------
	
	FileTree::FileTree()
	{
		_data = RN::FileManager::GetSharedInstance()->GetSearchPaths()->Retain();
		
		_tree = new DraggableOutlineView();
		_tree->SetAutoresizingMask(RN::UI::View::AutoresizingMask::FlexibleHeight | RN::UI::View::AutoresizingMask::FlexibleWidth);
		_tree->SetDataSource(this);
		_tree->SetDelegate(this);
		
		// Expand the first level of items
		_tree->ReloadData();
		_tree->ExpandItem(nullptr, false);
		
		AddSubview(_tree);
	}
	
	FileTree::~FileTree()
	{
		_tree->Release();
		_data->Release();
	}
	
	
	// -----------------------
	// MARK: -
	// MARK: Drag & Drop
	// -----------------------
	
	bool FileTree::CanDragItemAtRow(DraggableOutlineView *outlineView, void *item, size_t row)
	{
		RN::FileSystemNode *node = static_cast<RN::FileSystemNode *>(item);
		return node->IsKindOfClass(RN::FileProxy::GetMetaClass());
	}
	
	RN::Object *FileTree::GetObjectForDraggedItem(DraggableOutlineView *outlineView, void *item)
	{
		try
		{
			RN::FileSystemNode *node = static_cast<RN::FileSystemNode *>(item);
			std::string path = RN::FileManager::GetSharedInstance()->GetNormalizedPathFromFullpath(node->GetPath());
			
			RN::Object *object = RN::ResourceCoordinator::GetSharedInstance()->GetResourceWithName<RN::Object>(RNSTR(path.c_str()), nullptr);
			return object;
		}
		catch(RN::Exception &)
		{
			return nullptr;
		}
	}
	
	// -----------------------
	// MARK: -
	// MARK: RN::UI::OutlineViewDataSource
	// -----------------------
	
	RN::Array *FileTree::GetFilteredDirectoryProxy(RN::DirectoryProxy *proxy)
	{
		return proxy->GetSubNodes();
	}
	
	bool FileTree::OutlineViewItemIsExpandable(RN::UI::OutlineView *outlineView, void *item)
	{
		RN::FileSystemNode *node = static_cast<RN::FileSystemNode *>(item);
		return node->IsDirectory();
	}
	
	size_t FileTree::OutlineViewGetNumberOfChildrenForItem(RN::UI::OutlineView *outlineView, void *item)
	{
		if(!item)
			return _data->GetCount();
		
		RN::FileSystemNode *fsnode = static_cast<RN::FileSystemNode *>(item);
		if(!fsnode->IsDirectory())
			return 0;
		
		RN::DirectoryProxy *node = static_cast<RN::DirectoryProxy *>(item);
		RN::Array *children = GetFilteredDirectoryProxy(node);
		
		return children->GetCount();
	}
	
	void *FileTree::OutlineViewGetChildOfItem(RN::UI::OutlineView *outlineView, void *item, size_t child)
	{
		if(!item)
			return _data->GetObjectAtIndex(child);
		
		RN::DirectoryProxy *node = static_cast<RN::DirectoryProxy *>(item);
		RN::Array *children = GetFilteredDirectoryProxy(node);
		
		return children->GetObjectAtIndex(child);
	}
	
	RN::UI::OutlineViewCell *FileTree::OutlineViewGetCellForItem(RN::UI::OutlineView *outlineView, void *item)
	{
		RN::String *identifier = RNCSTR("Cell");
		DraggableOutlineViewCell *cell = static_cast<DraggableOutlineViewCell *>(outlineView->DequeueCellWithIdentifier(identifier));
		
		if(!cell)
		{
			cell = new DraggableOutlineViewCell(_tree, identifier);
			cell->Autorelease();
		}
		
		RN::FileSystemNode *node = static_cast<RN::FileSystemNode *>(item);
		
		cell->GetTextLabel()->SetText(RNSTR(node->GetName().c_str()));
		
		return cell;
	}
}
