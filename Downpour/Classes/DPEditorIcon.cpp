//
//  DPEditorIcon.cpp
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

#include "DPEditorIcon.h"
#include "DPWorkspace.h"

#define kDPEditorIconAssociationKey "kDPEditorIconAssociationKey"

namespace DP
{
	RNDefineMeta(EditorIcon, RN::Billboard)
	
	EditorIcon::EditorIcon(RN::SceneNode *node) :
		_shadowed(node)
	{
		_camera = Workspace::GetSharedInstance()->GetViewport()->GetCamera();
		_shadowed->SetAssociatedObject(kDPEditorIconAssociationKey, this, RN::Object::MemoryPolicy::Retain);
		
		SetFlags(GetFlags() | RN::SceneNode::Flags::HideInEditor | RN::SceneNode::Flags::NoSave);
		SetRenderGroup(31);
		SetCollisionGroup(31);
		GetMaterial()->SetDiscard(true);
		
		
		if(node->IsKindOfClass(RN::Light::MetaClass()))
			SetIcon("light.png");
		
		if(node->IsKindOfClass(RN::Camera::MetaClass()))
			SetIcon("camera.png");
		
		RN::MessageCenter::GetSharedInstance()->AddObserver(RNCSTR("DPClose"), [this](RN::Message *message) {
			_shadowed->RemoveAssociatedOject(kDPEditorIconAssociationKey);
			RemoveFromWorld();
		}, this);
	}
	
	EditorIcon::~EditorIcon()
	{
		RN::MessageCenter::GetSharedInstance()->RemoveObserver(this, RNCSTR("DPClose"));
	}
	
	
	void EditorIcon::SetIcon(const std::string &name)
	{
		std::string resources = Workspace::GetSharedInstance()->GetResourcePath();
		std::string path = RN::PathManager::Join(resources, RN::PathManager::Join("icons", name));
		
		RN::Texture::Parameter parameter;
		parameter.filter = RN::Texture::Filter::Nearest;
		
		SetTexture(RN::Texture::WithFile(path, parameter), 0.035f);
	}
	
	void EditorIcon::UpdateEditMode(float delta)
	{
		RN::Billboard::UpdateEditMode(delta);
		
		SetPosition(_shadowed->GetWorldPosition());
		SetWorldRotation(_camera->GetWorldRotation());
	}
	
	
	EditorIcon *EditorIcon::WithSceneNode(RN::SceneNode *node)
	{
		EditorIcon *icon = new EditorIcon(node);
		return icon->Autorelease();
	}
	
	bool EditorIcon::SupportsSceneNodeClass(RN::SceneNode *node)
	{
		if(node->IsKindOfClass(RN::Light::MetaClass()))
			return true;
		
		if(node->IsKindOfClass(RN::Camera::MetaClass()))
			return true;
		
		return false;
	}
}
