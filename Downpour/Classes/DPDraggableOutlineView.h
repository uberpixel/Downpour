//
//  DPDraggableOutlineView.h
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

#ifndef __DPDRAGGABLEOUTLINEVIEW_H__
#define __DPDRAGGABLEOUTLINEVIEW_H__

#include <Rayne/Rayne.h>
#include "DPColorScheme.h"

namespace DP
{
	class DraggableOutlineViewCell;
	class DraggableOutlineView : public RN::UI::OutlineView
	{
	public:
		friend class DraggableOutlineViewCell;
		
		struct Delegate : public RN::UI::OutlineViewDelegate
		{
			virtual bool CanDragItemAtRow(DraggableOutlineView *outlineView, void *item, size_t row) = 0;
			virtual RN::Object *GetObjectForDraggedItem(DraggableOutlineView *outlineView, void *item) = 0;
		};
		
		DraggableOutlineView();
		
		void SetDelegate(Delegate *delegate);
		
	private:
		void MouseDownOnCell(RN::UI::OutlineViewCell *cell, RN::Event *event);
		void MouseDraggedOnCell(RN::UI::OutlineViewCell *cell, RN::Event *event);
		void MouseUpOnCell(RN::UI::OutlineViewCell *cell, RN::Event *event);
		
		void *_draggedItem;
		size_t _draggedRow;
		
		Delegate *_delegate;
		
		RNDeclareMeta(DraggableOutlineView)
	};
	
	class DraggableOutlineViewCell : public OutlineViewCell
	{
	public:
		DraggableOutlineViewCell(DraggableOutlineView *tree, RN::String *identifier) :
			OutlineViewCell(identifier),
			_tree(tree)
		{}
		
		
		
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
		DraggableOutlineView *_tree;
		
		RNDeclareMeta(DraggableOutlineViewCell)
	};
}

#endif /* __DPDRAGGABLEOUTLINEVIEW_H__ */
