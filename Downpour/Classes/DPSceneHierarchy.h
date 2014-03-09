//
//  DPSceneHierarchy.h
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

#ifndef __DPSCENEHIERARCHY_H__
#define __DPSCENEHIERARCHY_H__

#include <Rayne/Rayne.h>

namespace DP
{
	struct SceneNodeProxy
	{
		SceneNodeProxy(RN::SceneNode *tnode) :
			node(tnode)
		{
			if(node->HasChildren())
			{
				const RN::Array *temp = node->GetChildren();
				temp->Enumerate<RN::SceneNode>([&](RN::SceneNode *child, size_t index, bool &stop) {
					
					if(child->GetFlags() & RN::SceneNode::Flags::HideInEditor)
						return;
					
					children.emplace_back(new SceneNodeProxy(child));
				});
			}
		}
		
		~SceneNodeProxy()
		{
			for(SceneNodeProxy *proxy : children)
			{
				delete proxy;
			}
		}
		
		SceneNodeProxy *FindProxyForNode(RN::SceneNode *tnode)
		{
			if(tnode == node)
				return this;
			
			for(SceneNodeProxy *proxy : children)
			{
				SceneNodeProxy *temp = proxy->FindProxyForNode(tnode);
				if(temp)
					return temp;
			}
			
			return nullptr;
		}
		
		bool RemoveProxyForNode(RN::SceneNode *tnode)
		{
			for(auto i = children.begin(); i != children.end(); i ++)
			{
				SceneNodeProxy *proxy = *i;
				
				if(proxy->node == tnode)
				{
					delete proxy;
					children.erase(i);
					
					return true;
				}
			}
			
			for(SceneNodeProxy *proxy : children)
			{
				bool result = proxy->RemoveProxyForNode(tnode);
				if(result)
					return result;
			}
			
			return false;
		}
		
		bool IsExpandable() const { return !children.empty(); }
		
		RN::SceneNode *node;
		std::vector<SceneNodeProxy *> children;
	};
	
	class SceneHierarchy : public RN::UI::View, RN::UI::OutlineViewDataSource, RN::UI::OutlineViewDelegate
	{
	public:
		SceneHierarchy();
		~SceneHierarchy();
		
		void DidAddSceneNode(RN::Message *message);
		void WillRemoveSceneNode(RN::Message *message);
		
	private:
		SceneNodeProxy *FindProxyForNode(RN::SceneNode *node);
		
		bool OutlineViewItemIsExpandable(RN::UI::OutlineView *outlineView, void *item) override;
		size_t OutlineViewGetNumberOfChildrenForItem(RN::UI::OutlineView *outlineView, void *item) override;
		void *OutlineViewGetChildOfItem(RN::UI::OutlineView *outlineView, void *item, size_t child) override;
		RN::UI::OutlineViewCell *OutlineViewGetCellForItem(RN::UI::OutlineView *outlineView, void *item) override;
		
		void OutlineViewSelectionDidChange(RN::UI::OutlineView *outlineView) override;
		
		std::vector<SceneNodeProxy *> _data;
		
		RN::UI::OutlineView *_tree;
		bool _suppressSelectionNotification;
	};
}

#endif /* __DPSCENEHIERARCHY_H__ */
