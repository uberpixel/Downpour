//
//  DPWorldAttachment.cpp
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

#include "DPWorldAttachment.h"
#include "DPWorkspace.h"

namespace DP
{
	WorldAttachment::WorldAttachment(RN::Camera *camera) :
		_sceneNodes(nullptr),
		_camera(camera)
	{
		_lightClass  = RN::Light::MetaClass();
		_cameraClass = RN::Camera::MetaClass();
		
		RN::MessageCenter::GetSharedInstance()->AddObserver(kDPWorkspaceSelectionChanged, [this](RN::Message *message) {
			
			RN::SafeRelease(_sceneNodes);
			RN::Array *objects = static_cast<RN::Array *>(message->GetObject());
			
			_sceneNodes = objects ? new RN::Set(objects) : new RN::Set();
			
		}, this);
	}
	
	WorldAttachment::~WorldAttachment()
	{
		RN::SafeRelease(_sceneNodes);
		RN::MessageCenter::GetSharedInstance()->RemoveObserver(this);
	}
	
	
	
	void WorldAttachment::DidBeginCamera(RN::Camera *camera)
	{
		_shouldDraw = (_camera == camera && _sceneNodes && _sceneNodes->GetCount() > 0);
	}
	
	void WorldAttachment::WillRenderSceneNode(RN::SceneNode *node)
	{
		if(_shouldDraw && _sceneNodes->ContainsObject(node))
		{
			// Cameras aren't drawn yet
			if(node->IsKindOfClass(_cameraClass))
				return;
			
			if(node->IsKindOfClass(_lightClass))
			{
				float distance = _camera->GetWorldPosition().GetDistance(node->GetWorldPosition());
				float tessellation = ((_camera->GetClipFar() - distance) / _camera->GetClipFar()) * 30.0f;
				
				tessellation = std::max(5.0f, tessellation);
				
				RN::Debug::DrawSphere(node->GetBoundingSphere(), RN::Color::Yellow(), static_cast<int>(floorf(tessellation)));
			}
			else
			{
				RN::Debug::DrawBox(node->GetBoundingBox(), RN::Color::Red());
			}
		}
	}
}
