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

const char *kDPFileTreeChangesAssociationKey = "kDPFileTreeChangesAssociationKey";

namespace DP
{
	// -----------------------
	// MARK: -
	// MARK: FileTreeCell
	// -----------------------
	
	class FileTreeCell : public RN::UI::OutlineViewCell
	{
	public:
		FileTreeCell(RN::String *identifier) :
			RN::UI::OutlineViewCell(identifier)
		{
			SetSelected(IsSelected());
		}
		
		void SetSelected(bool selected) override
		{
			RN::UI::OutlineViewCell::SetSelected(selected);
			
			if(selected)
			{
				SetBackgroundColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_Selection));
				GetTextLabel()->SetTextColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_TextSelection));
			}
			else
			{
				SetBackgroundColor(RN::Color::ClearColor());
				GetTextLabel()->SetTextColor(ColorScheme::GetColor(ColorScheme::Type::FileTree_Text));
			}
		}
	};
	
	
	// -----------------------
	// MARK: -
	// MARK: FileTree
	// -----------------------
	
	FileTree::FileTree() :
		RN::UI::View(RN::Rect(0.0f, 0.0f, 250.0f, 0.0f))
	{
		_data = RN::FileManager::GetSharedInstance()->GetSearchPaths()->Retain();
		
		_tree = new RN::UI::OutlineView();
		_tree->SetFrame(GetBounds());
		_tree->SetAutoresizingMask(RN::UI::View::AutoresizingFlexibleHeight);
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
			cell = new FileTreeCell(identifier);
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
