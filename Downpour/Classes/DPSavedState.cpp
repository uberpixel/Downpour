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

/* ------------------
 
 The SavedState is responsible for capturing the current (unknown) state of the scene,
 and transforming it into a state that is known to Downpour, while allowing to restore the
 old state when Downpour is closed again.
 
   ------------------ */

namespace DP
{
	SavedState::SavedState() :
		_mainCamera(nullptr),
		_cameras(new RN::Array()),
		_lights(new RN::Array())
	{
		RN::Array *sceneGraph = RN::World::GetActiveWorld()->GetSceneNodes();
		
		RN::MetaClassBase *cameraClass = RN::Camera::MetaClass();
		RN::MetaClassBase *lightClass  = RN::Light::MetaClass();
		
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
		_cameras->Enumerate<RN::Camera>([&](RN::Camera *camera, size_t index, bool &stop) {
			
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
			sceneGraph->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop) {
				
				if(node->IsKindOfClass(lightClass))
				{
					RN::Light *light = static_cast<RN::Light *>(node);
					
					if(light->HasShadows() && light->GetShadowParameters().shadowTarget == _mainCamera)
						_lights->AddObject(light);
				}
				
			});
		}
		
		// Misc
		_maxFPS = RN::Kernel::GetSharedInstance()->GetMaxFPS();
		RN::Kernel::GetSharedInstance()->SetMaxFPS(20);
	}
	
	SavedState::~SavedState()
	{
		_cameras->Enumerate<RN::Camera>([&](RN::Camera *camera, size_t index, bool &stop) {
			RN::Camera::Flags flags = camera->GetFlags();
			camera->SetFlags(flags & ~RN::Camera::Flags::Hidden);
		});
		_cameras->Release();
		
		_lights->Enumerate<RN::Light>([&](RN::Light *light, size_t index, bool &stop) {
			
			RN::ShadowParameter parameter = light->GetShadowParameters();
			parameter.shadowTarget = _mainCamera;
			
			light->UpdateShadowParameters(parameter);
			
		});
		_lights->Release();
		
		RN::Kernel::GetSharedInstance()->SetMaxFPS(_maxFPS);
	}
}
