//
//  DPViewport.cpp
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

#include "DPViewport.h"
#include "DPWorkspace.h"

namespace DP
{
	Viewport::Viewport()
	{
		SetInteractionEnabled(true);
		
		_camera = new RN::Camera(RN::Vector2(), RN::Texture::Format::RGB16F, RN::Camera::Flags::UpdateAspect | RN::Camera::Flags::UpdateStorageFrame | RN::Camera::Flags::NoFlush);
		_camera->SetBlitShader(RN::Shader::WithFile("shader/rn_DrawFramebufferTonemap"));
		
		_sourceCamera = Workspace::GetSharedInstance()->GetSavedState()->GetMainCamera();
		if(_sourceCamera)
		{
			_camera->SetPosition(_sourceCamera->GetWorldPosition());
			_camera->SetRotation(_sourceCamera->GetWorldRotation());
			
			_camera->SetRenderGroups(_sourceCamera->GetRenderGroups());
			_camera->SetSky(_sourceCamera->GetSky());
			
			RN::Array *lights = Workspace::GetSharedInstance()->GetSavedState()->GetLights();
			lights->Enumerate<RN::Light>([&](RN::Light *light, size_t index, bool &stop) {
					
				RN::ShadowParameter paramter = light->GetShadowParameters();
				paramter.shadowTarget = _camera;
				
				light->UpdateShadowParameters(paramter);
				
			});
		}
		
		_renderView = new RenderView();
		_renderView->SetAutoresizingMask(RN::UI::View::AutoresizingFlexibleHeight | RN::UI::View::AutoresizingFlexibleWidth);
		_renderView->SetTexture(_camera->GetStorage()->GetRenderTarget());
		
		AddSubview(_renderView);
	}
	
	Viewport::~Viewport()
	{
		_camera->Release();
		_renderView->Release();
	}
	
	
	void Viewport::SetFrame(const RN::Rect &frame)
	{
		RN::UI::View::SetFrame(frame);
		_camera->SetFrame(RN::Rect(RN::Vector2(), frame.Size()));
	}
	
	
	void Viewport::Update()
	{
		RN::UI::View::Update();
		
		if(GetWidget()->GetFirstResponder() == this)
		{
			RN::Vector3 translation;
			RN::Input *input = RN::Input::GetSharedInstance();
			
			translation.x = (input->IsKeyPressed('d') - input->IsKeyPressed('a')) * 16.0f;
			translation.z = (input->IsKeyPressed('s') - input->IsKeyPressed('w')) * 16.0f;
			
			translation *= (input->GetModifierKeys() & RN::KeyModifier::KeyShift) ? 2.0f : 1.0f;
			
			_camera->TranslateLocal(translation * RN::Kernel::GetSharedInstance()->GetDelta());
		}
		
		if(_sourceCamera)
		{
			_camera->SetAmbientColor(_sourceCamera->GetAmbientColor());
			_camera->SetFogColor(_sourceCamera->GetFogColor());
		}
	}
	
	bool Viewport::CanBecomeFirstResponder()
	{
		return true;
	}
	
	void Viewport::MouseDown(RN::Event *event)
	{
		GetWidget()->MakeFirstResponder(this);
	}
	
	void Viewport::MouseDragged(RN::Event *event)
	{
		if(event->GetButton() == 1)
		{
			const RN::Vector2 &delta = event->GetMouseDelta();
			_camera->Rotate(RN::Vector3(delta.x, delta.y, 0.0f));
		}
	}
}
