//
//  DPDraggableOutlineView.cpp
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

#include "DPDraggableOutlineView.h"
#include "DPDragNDropTarget.h"

namespace DP
{
	RNDefineMeta(DraggableOutlineView, RN::UI::OutlineView)
	RNDefineMeta(DraggableOutlineViewCell, OutlineViewCell)
	
	DraggableOutlineView::DraggableOutlineView() :
		_draggedItem(nullptr),
		_delegate(nullptr)
	{
	}
	
	void DraggableOutlineView::SetDelegate(Delegate *delegate)
	{
		RN::UI::OutlineView::SetDelegate(delegate);
		_delegate = delegate;
	}
	
	void DraggableOutlineView::MouseDownOnCell(RN::UI::OutlineViewCell *cell, RN::Event *event)
	{
		if(!_delegate)
			return;
		
		RN::Range range = GetVisibleRange();
		size_t row = kRNNotFound;
		
		for(size_t i = 0; i < range.length; i ++)
		{
			RN::UI::TableViewCell *temp = GetCellForRow(i + range.origin);
			if(temp == cell)
			{
				row = i + range.origin;
				break;
			}
		}
		
		if(row != kRNNotFound)
		{
			void *item = GetItemForRow(row);
			
			if(_delegate->CanDragItemAtRow(this, item, row))
			{
				_draggedItem = item;
				_draggedRow  = row;
				
				RN::IndexSet *selection = (new RN::IndexSet(_draggedRow))->Autorelease();
				SetSelection(selection);
			}
		}
	}
	
	void DraggableOutlineView::MouseDraggedOnCell(RN::UI::OutlineViewCell *cell, RN::Event *event)
	{
	}
	
	void DraggableOutlineView::MouseUpOnCell(RN::UI::OutlineViewCell *cell, RN::Event *event)
	{
		if(_draggedItem)
		{
			RN::UI::View *base = GetWidget()->GetContentView();
			RN::UI::View *hit  = base->HitTest(base->ConvertPointFromBase(event->GetMousePosition()), event);
			
			if(hit->IsKindOfClass(DragNDropTarget::MetaClass()))
			{
				DragNDropTarget *target = static_cast<DragNDropTarget *>(hit);
				RN::Object *object = _delegate->GetObjectForDraggedItem(this, _draggedItem);
				
				if(target->AcceptsDropOfObject(object))
				{
					RN::Vector2 position = target->ConvertPointFromBase(event->GetMousePosition());
					target->HandleDropOfObject(object, position);
				}
			}
			
			_draggedItem = nullptr;
		}
	}
}
