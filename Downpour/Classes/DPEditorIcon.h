//
//  DPEditorIcon.h
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

#ifndef __DPEDITORICON_H__
#define __DPEDITORICON_H__

#include <Rayne/Rayne.h>

namespace DP
{
	class EditorIcon : public RN::Billboard
	{
	public:
		EditorIcon(RN::SceneNode *node);
		~EditorIcon();
		
		void Detach();
		void UpdateEditMode(float delta) override;
		RN::SceneNode *GetSceneNode() const { return _shadowed; }
		
		static bool SupportsSceneNodeClass(RN::SceneNode *node);
		
		static EditorIcon *WithSceneNode(RN::SceneNode *node);
		static EditorIcon *GetIconForSceneNode(RN::SceneNode *node);
		
	private:
		void SetIcon(const std::string &name);
		
		RN::SceneNode *_shadowed;
		RN::Camera *_camera;
		
		RNDeclareMeta(EditorIcon)
	};
}

#endif /* __DPEDITORICON_H__ */
