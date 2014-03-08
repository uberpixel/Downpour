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
	// MARK: FileTreeCell
	// -----------------------
	
	class FileTreeCell : public OutlineViewCell
	{
	public:
		FileTreeCell(FileTree *tree, RN::String *identifier) :
			OutlineViewCell(identifier),
			_tree(tree)
		{}
		
		
		// Drag & Drop
		void MouseDown(RN::Event *event) override
		{
			OutlineViewCell::MouseDown(event);
			_tree->MouseDownOnCell(this, event);
		}
		
		void MouseDragged(RN::Event *event) override
		{
			OutlineViewCell::MouseDragged(event);
			_tree->MouseDraggedOnCell(this, event);
		}
		
		void MouseUp(RN::Event *event) override
		{
			OutlineViewCell::MouseUp(event);
			_tree->MouseUpOnCell(this, event);
		}
		
	private:
		FileTree *_tree;
	};
	
	// -----------------------
	// MARK: -
	// MARK: FileTree
	// -----------------------
	
	FileTree::FileTree() :
		_draggedNode(nullptr)
	{
		_data = RN::FileManager::GetSharedInstance()->GetSearchPaths()->Retain();
		
		_tree = new RN::UI::OutlineView();
		_tree->SetAutoresizingMask(RN::UI::View::AutoresizingFlexibleHeight | RN::UI::View::AutoresizingFlexibleWidth);
		_tree->SetDataSource(this);
		_tree->SetDelegate(this);
		
		// Expand the first level of items
		_tree->ReloadData();
		_tree->ExpandItem(nullptr, false);
		
		AddSubview(_tree);
		SetBackgroundColor(ColorScheme::GetColor(ColorScheme::Type::Background));
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
	
	void FileTree::MouseDownOnCell(RN::UI::OutlineViewCell *cell, RN::Event *event)
	{
		RN::Range range = _tree->GetVisibleRange();
		size_t row = kRNNotFound;
		
		for(size_t i = 0; i < range.length; i ++)
		{
			RN::UI::TableViewCell *temp = _tree->GetCellForRow(i + range.origin);
			if(temp == cell)
			{
				row = i + range.origin;
				break;
			}
		}
		
		if(row != kRNNotFound)
		{
			RN::FileSystemNode *node = static_cast<RN::FileSystemNode *>(_tree->GetItemForRow(row));
			
			if(node->IsKindOfClass(RN::FileProxy::MetaClass()))
			{
				RN::IndexSet *selection = new RN::IndexSet(row);
				
				try
				{
					std::string path = RN::FileManager::GetSharedInstance()->GetNormalizedPathFromFullpath(node->GetPath());
					
					_draggedData = RN::ResourceCoordinator::GetSharedInstance()->GetResourceWithName<RN::Object>(RNSTR(path.c_str()), nullptr);
					_draggedData->Retain();
					
					_draggedNode = static_cast<RN::FileProxy *>(node);
					_tree->SetSelection(selection->Autorelease());
				}
				catch(RN::Exception &e)
				{
					_draggedData = nullptr;
					_draggedNode = nullptr;
				}
			};
		}
	}
	
	void FileTree::MouseDraggedOnCell(RN::UI::OutlineViewCell *cell, RN::Event *event)
	{
		if(_draggedNode)
		{
			
		}
	}
	
	void FileTree::MouseUpOnCell(RN::UI::OutlineViewCell *cell, RN::Event *event)
	{
		if(_draggedNode)
		{
			RN::UI::View *base = GetWidget()->GetContentView();
			RN::UI::View *hit  = base->HitTest(base->ConvertPointFromBase(event->GetMousePosition()), event);
			
			if(hit->IsKindOfClass(DragNDropTarget::MetaClass()))
			{
				DragNDropTarget *target = static_cast<DragNDropTarget *>(hit);
				
				if(target->AcceptsDropOfObject(_draggedData))
				{
					RN::Vector2 position = target->ConvertPointFromBase(event->GetMousePosition());
					target->HandleDropOfObject(_draggedData, position);
				}
			}
			
			_draggedData->Release();
			_draggedData = nullptr;
			_draggedNode = nullptr;
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
		FileTreeCell *cell = static_cast<FileTreeCell *>(outlineView->DequeCellWithIdentifier(identifier));
		
		if(!cell)
		{
			cell = new FileTreeCell(this, identifier);
			cell->Autorelease();
		}
		
		RN::FileSystemNode *node = static_cast<RN::FileSystemNode *>(item);
		
		cell->GetTextLabel()->SetText(RNSTR(node->GetName().c_str()));
		
		return cell;
	}
	
	
	// -----------------------
	// MARK: -
	// MARK: RN::UI::OutlineViewDelegate
	// -----------------------
	
	void FileTree::OutlineViewDidSelectItem(RN::UI::OutlineView *outlineView, void *item)
	{
		RN::FileSystemNode *file =  static_cast<RN::FileSystemNode*>(item);
		
		if(!file->IsFile())
			return;
		
		// TODO: Functionality for drag and drop of files
	}
}
