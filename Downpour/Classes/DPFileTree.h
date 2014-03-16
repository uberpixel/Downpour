//
//  DPFileTree.h
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

#ifndef __DPFILETREE_H__
#define __DPFILETREE_H__

#include <Rayne/Rayne.h>
#include "DPDraggableOutlineView.h"

namespace DP
{
	class FileTree : public RN::UI::View, RN::UI::OutlineViewDataSource, DraggableOutlineView::Delegate
	{
	public:
		FileTree();
		~FileTree() override;
		
		bool CanDragItemAtRow(DraggableOutlineView *outlineView, void *item, size_t row);
		RN::Object *GetObjectForDraggedItem(DraggableOutlineView *outlineView, void *item);
		
	private:
		RN::Array *GetFilteredDirectoryProxy(RN::DirectoryProxy *proxy);
		
		bool OutlineViewItemIsExpandable(RN::UI::OutlineView *outlineView, void *item) override;
		size_t OutlineViewGetNumberOfChildrenForItem(RN::UI::OutlineView *outlineView, void *item) override;
		void *OutlineViewGetChildOfItem(RN::UI::OutlineView *outlineView, void *item, size_t child) override;
		RN::UI::OutlineViewCell *OutlineViewGetCellForItem(RN::UI::OutlineView *outlineView, void *item) override;
		
		RN::Array *_data;
		DraggableOutlineView *_tree;
	};
}

#endif /* __DPFILETREE_H__ */
