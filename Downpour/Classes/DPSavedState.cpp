//
//  DPSavedState.cpp
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

#include "DPSavedState.h"
#include "DPWorldAttachment.h"

/* ------------------
 
 The SavedState is responsible for capturing the current (unknown) state of the scene,
 and transforming it into a state that is known to Downpour, while allowing to restore the
 old state when Downpour is closed again.
 
   ------------------ */

namespace DP
{
	SavedState::SavedState() :
		_mainCamera(nullptr),
		_cameras(new RN::Set()),
		_lights(new RN::Set()),
		_instancingNodes(new RN::Set())
	{
		RN::Array *sceneGraph = RN::World::GetActiveWorld()->GetSceneNodes();
		
		RN::MetaClassBase *cameraClass = RN::Camera::MetaClass();
		RN::MetaClassBase *lightClass  = RN::Light::MetaClass();
		RN::MetaClassBase *instancingClass  = RN::InstancingNode::MetaClass();
		
		// Disable all cameras to prevent them from flushing onto the screen while the editor is active
		sceneGraph->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop) {
			
			if(node->IsKindOfClass(cameraClass))
			{
				RN::Camera *camera = static_cast<RN::Camera *>(node);
				RN::Camera::Flags flags = camera->GetFlags();
				
				if((flags & RN::Camera::Flags::NoRender) || ((flags & RN::Camera::Flags::NoFlush) && camera->GetPostProcessingPipelines().empty()))
					return;
				
				_cameras->AddObject(node);
				camera->SetFlags(flags | RN::Camera::Flags::Hidden);
			}
			
		});
		
		// Take an educated guess about what the "main" rendering camera might be
		_cameras->Enumerate<RN::Camera>([&](RN::Camera *camera, bool &stop) {
			
			if(_mainCamera)
			{
				if(camera->GetPriority() > _mainCamera->GetPriority())
				{
					_mainCamera = camera;
					return;
				}
			}
			else
			{
				_mainCamera = camera;
			}
			
		});
		
		// Find all cameras that use the main camera as shadow target
		if(_mainCamera)
		{
			_mainCamera->SceneNode::SetFlags(_mainCamera->SceneNode::GetFlags() | RN::SceneNode::Flags::LockedInEditor);
			
			sceneGraph->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop) {
				
				if(node->IsKindOfClass(lightClass))
				{
					RN::Light *light = static_cast<RN::Light *>(node);
					
					if(light->HasShadows() && light->GetShadowParameters().shadowTarget == _mainCamera)
						_lights->AddObject(light);
				}
				
				if(node->IsKindOfClass(instancingClass))
				{
					RN::InstancingNode *inode = static_cast<RN::InstancingNode *>(node);
					
					if(inode->GetPivot() == _mainCamera)
						_instancingNodes->AddObject(inode);
				}
				
			});
		}
		
		// Misc
		_maxFPS = RN::Kernel::GetSharedInstance()->GetMaxFPS();
		RN::Kernel::GetSharedInstance()->SetMaxFPS(20);
		RN::World::GetActiveWorld()->SetMode(RN::World::Mode::Edit);
		
		// Listen to removal of scene nodes
		RN::MessageCenter::GetSharedInstance()->AddObserver(kDPWorldAttachmentWillRemoveSceneNode, [this](RN::Message *message) {
			
			RN::Object *node = message->GetObject();
			if(_cameras->ContainsObject(node))
			{
				_cameras->RemoveObject(node);
				return;
			}
			
			if(_lights->ContainsObject(node))
			{
				_lights->RemoveObject(node);
				return;
			}
			
			if(_instancingNodes->ContainsObject(node))
			{
				_instancingNodes->RemoveObject(node);
				return;
			}
			
			// XXX: Todo: What about the main camera?
			// Removing it should work, but that also requires the viewport to play nicely
			
		}, this);
	}
	
	SavedState::~SavedState()
	{
		RN::MessageCenter::GetSharedInstance()->RemoveObserver(this);
		
		_cameras->Enumerate<RN::Camera>([&](RN::Camera *camera, bool &stop) {
			RN::Camera::Flags flags = camera->GetFlags();
			camera->SetFlags(flags & ~RN::Camera::Flags::Hidden);
		});
		_cameras->Release();
		
		if(_mainCamera)
		{
			UpdateCamera(_mainCamera);
			
			_mainCamera->SceneNode::SetFlags(_mainCamera->SceneNode::GetFlags() & ~RN::SceneNode::Flags::LockedInEditor);
		}
		
		_lights->Release();
		_instancingNodes->Release();
		
		RN::Kernel::GetSharedInstance()->SetMaxFPS(_maxFPS);
		RN::World::GetActiveWorld()->SetMode(RN::World::Mode::Play);
	}
	
	
	void SavedState::UpdateCamera(RN::Camera *newCamera)
	{
		_lights->Enumerate<RN::Light>([&](RN::Light *light, bool &stop) {
			
			RN::ShadowParameter paramter = light->GetShadowParameters();
			paramter.shadowTarget = newCamera;
			
			light->UpdateShadowParameters(paramter);
			
		});
		
		_instancingNodes->Enumerate<RN::InstancingNode>([&](RN::InstancingNode *node, bool &stop) {
			
			node->SetPivot(newCamera);
			
		});
	}
}
