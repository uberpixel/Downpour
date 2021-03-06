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
		
		RN::Camera::Flags flags = RN::Camera::Flags::UpdateAspect | RN::Camera::Flags::UpdateStorageFrame | RN::Camera::Flags::NoFlush;
		
		_camera = new RN::Camera(RN::Vector2(32.0f), RN::Texture::Format::RGB16F, flags);
		_camera->GetStorage()->SetDepthTarget(RN::Texture::Format::DepthStencil);
		_camera->SceneNode::SetFlags(_camera->SceneNode::GetFlags() | RN::SceneNode::Flags::HideInEditor | RN::SceneNode::Flags::NoSave);
		
		_editorCamera = new RN::Camera(RN::Vector2(32.0f), RN::Texture::Format::RGBA16F, flags);
		_editorCamera->GetStorage()->SetDepthTarget(RN::Texture::Format::DepthStencil);
		_editorCamera->SetRenderGroups((1 << 31));
		_editorCamera->SetClearColor(RN::Color::ClearColor());
		_editorCamera->SceneNode::SetFlags(_editorCamera->SceneNode::GetFlags() | RN::SceneNode::Flags::HideInEditor | RN::SceneNode::Flags::NoSave);
		
		_sourceCamera = Workspace::GetSharedInstance()->GetSavedState()->GetMainCamera();
		if(_sourceCamera)
		{
			_camera->SetPosition(_sourceCamera->GetWorldPosition());
			_camera->SetRotation(_sourceCamera->GetWorldRotation());
			
			_camera->SetRenderGroups(_sourceCamera->GetRenderGroups());
			_camera->SetSky(_sourceCamera->GetSky());
			
			Workspace::GetSharedInstance()->GetSavedState()->UpdateCamera(_camera);
		}
		
		// Post processing chain to alter editor content when occluded
		{
			std::string path = RN::PathManager::Join(Workspace::GetSharedInstance()->GetResourcePath(), "shaders");
			
			RN::Shader *shader = new RN::Shader();
			shader->SetShaderForType(RN::PathManager::Join(path, "PPViewport.vsh"), RN::ShaderType::VertexShader);
			shader->SetShaderForType(RN::PathManager::Join(path, "PPViewport.fsh"), RN::ShaderType::FragmentShader);
			
			RN::Material *material = new RN::Material(shader->Autorelease());
			material->AddTexture(_camera->GetStorage()->GetDepthTarget());
			material->AddTexture(_editorCamera->GetStorage()->GetDepthTarget());
			
			_postProcessCamera = new RN::Camera(RN::Vector2(32.0f), RN::Texture::Format::RGBA16F, RN::Camera::Flags::UpdateStorageFrame, RN::RenderStorage::BufferFormatColor);
			_postProcessCamera->SetClearColor(RN::Color::ClearColor());
			_postProcessCamera->SetMaterial(material->Autorelease());
			_postProcessCamera->SceneNode::SetFlags(_editorCamera->SceneNode::GetFlags() | RN::SceneNode::Flags::HideInEditor | RN::SceneNode::Flags::NoSave);
			
			RN::PostProcessingPipeline *pipeline = _editorCamera->AddPostProcessingPipeline("pipeline", 3);
			pipeline->AddStage(_postProcessCamera, RN::RenderStage::Mode::ReUsePreviousStage);
		}
		
		
		_editorCamera->SetPosition(_camera->GetWorldPosition());
		_editorCamera->SetRotation(_camera->GetWorldRotation());
		
		
		_renderView = new RenderView();
		_renderView->SetAutoresizingMask(RN::UI::View::AutoresizingMask::FlexibleHeight | RN::UI::View::AutoresizingMask::FlexibleWidth);
		_renderView->AddTexture(_camera->GetRenderTarget());
		_renderView->AddTexture(_postProcessCamera->GetRenderTarget());
		
		AddSubview(_renderView);
		
		_resolutionFactor = RN::Settings::GetSharedInstance()->GetFloatForKey(RNCSTR("DPResolutionFactor"), 1.0f);
	}
	
	Viewport::~Viewport()
	{
		_camera->RemoveFromWorld();
		_camera->Release();
		
		_editorCamera->RemoveFromWorld();
		_editorCamera->Release();
		
		_postProcessCamera->RemoveFromWorld();
		_postProcessCamera->Release();
		
		_renderView->Release();
	}
	
	
	void Viewport::SetFrame(const RN::Rect &frame)
	{
		RN::UI::View::SetFrame(frame);
		
		RN::Rect rect = RN::Rect(RN::Vector2(), frame.GetSize() * _resolutionFactor);
		
		_camera->SetFrame(rect);
		_editorCamera->SetFrame(rect);
		_postProcessCamera->SetFrame(rect);
	}
	
	void Viewport::UpdateSourceCamera(RN::Camera *source)
	{
		_sourceCamera = source;
		
		if(_sourceCamera)
		{
			_camera->SetRenderGroups(_sourceCamera->GetRenderGroups());
			_camera->SetSky(_sourceCamera->GetSky());
		}
	}
	
	RN::Vector2 Viewport::ConvertPointToViewport(const RN::Vector2 &point)
	{
		return ConvertPointFromBase(point)*_resolutionFactor;
	}
	
	RN::Vector3 Viewport::GetDirectionForPoint(const RN::Vector2 &tpoint)
	{
		RN::Vector2 point = tpoint / _camera->GetFrame().GetSize();
		point.y = 1.0f - point.y;
		point *= 2.0f;
		point -= 1.0f;
		
		return (_camera->ToWorld(RN::Vector3(point, 1.0f)) - _camera->GetPosition()).Normalize();
	}
	
	RN::Vector3 Viewport::GetPositionForPoint(const RN::Vector2 &tpoint, float dist)
	{
		RN::Vector2 point = tpoint / _camera->GetFrame().GetSize();
		point.y = 1.0f - point.y;
		point *= 2.0f;
		point -= 1.0f;
		
		return _camera->ToWorld(RN::Vector3(point, dist));
	}
	
	RN::Vector3 Viewport::GetDirectionForMouse(const RN::Vector2 &point)
	{
		return GetDirectionForPoint(ConvertPointToViewport(point));
	}
	
	RN::Vector3 Viewport::GetPositionForMouse(const RN::Vector2 &point, float dist)
	{
		return GetPositionForPoint(ConvertPointToViewport(point), dist);
	}
	
	void Viewport::Update()
	{
		RN::UI::View::Update();
		
		if(GetWidget()->GetFirstResponder() == this)
		{
			RN::Vector3 translation;
			RN::Input *input = RN::Input::GetSharedInstance();
			
			if((input->GetModifierKeys() & ~RN::KeyModifier::KeyShift) == 0)
			{
				translation.x = (input->IsKeyPressed('d') - input->IsKeyPressed('a')) * 16.0f;
				translation.z = (input->IsKeyPressed('s') - input->IsKeyPressed('w')) * 16.0f;
				
				float vertical = (input->IsKeyPressed('e') - input->IsKeyPressed('q')) * 16.0f;
				
				translation *= (input->GetModifierKeys() & RN::KeyModifier::KeyShift) ? 2.0f : 1.0f;
				
				_camera->TranslateLocal(translation * RN::Kernel::GetSharedInstance()->GetDelta());
				_camera->Translate(RN::Vector3(0.0f, vertical, 0.0f) * RN::Kernel::GetSharedInstance()->GetDelta());
				
				_editorCamera->SetPosition(_camera->GetWorldPosition());
			}
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
		
		if(event->GetButton() == 0)
		{
			switch(Workspace::GetSharedInstance()->GetActiveTool())
			{
				case Workspace::Tool::Gizmo:
				{
					RN::Vector3 rayDirection = GetDirectionForMouse(event->GetMousePosition());
					RN::Vector3 rayPosition = GetPositionForMouse(event->GetMousePosition(), _camera->GetClipNear());
					RN::Hit hit;
					
					Workspace *workspace = Workspace::GetSharedInstance();
					Gizmo *gizmo = workspace->GetGizmo();
					
					
					if(gizmo->GetCollisionGroup() == 0)
						hit = std::move(gizmo->CastRay(rayPosition, rayDirection));
					
					if(hit.node == gizmo)
					{
						gizmo->BeginMove(hit.meshid, ConvertPointToViewport(event->GetMousePosition()));
						return;
					}
					
					hit = RN::World::GetActiveWorld()->GetSceneManager()->CastRay(rayPosition, rayDirection, (1 << 0) | (1 << 31));
					hit.node ? workspace->SetSelection(hit.node) : workspace->SetSelection(nullptr);
					
					break;
				}
					
				case Workspace::Tool::Sculpting:
				{
					SculptTool *sculptTool = Workspace::GetSharedInstance()->GetSculptTool();
					sculptTool->UseTool();
					break;
				}
			}
		}
	}
	
	void Viewport::MouseDragged(RN::Event *event)
	{
		if(event->GetButton() == 0)
		{
			switch(Workspace::GetSharedInstance()->GetActiveTool())
			{
				case Workspace::Tool::Gizmo:
				{
					Gizmo *gizmo = Workspace::GetSharedInstance()->GetGizmo();
					if(gizmo->IsActive())
					{
						RN::Vector2 mouse = ConvertPointToViewport(event->GetMousePosition());
						gizmo->ContinueMove(mouse);
					}
					break;
				}
				
				case Workspace::Tool::Sculpting:
				{
					SculptTool *sculptTool = Workspace::GetSharedInstance()->GetSculptTool();
					sculptTool->UseTool();
					break;
				}
			}
		}
		else
		if(event->GetButton() == 1)
		{
			const RN::Vector2 &delta = event->GetMouseDelta();
			
			_camera->Rotate(RN::Vector3(delta.x, delta.y, 0.0f));
			_editorCamera->SetRotation(_camera->GetWorldRotation());
		}
	}
	
	void Viewport::MouseMoved(RN::Event *event)
	{
		if(Workspace::GetSharedInstance()->GetActiveTool() == Workspace::Tool::Gizmo)
		{
			Gizmo *gizmo = Workspace::GetSharedInstance()->GetGizmo();

			RN::Vector3 direction = GetDirectionForMouse(event->GetMousePosition());
			RN::Hit hit;
			if(gizmo->GetCollisionGroup() == 0)
				hit = std::move(gizmo->CastRay(_camera->GetPosition(), direction));
				
			if(hit.node == gizmo)
			{
				gizmo->SetHighlight(hit.meshid);
			}
			else
			{
				gizmo->SetHighlight(-1);
			}
		}
	}
	
	void Viewport::MouseUp(RN::Event *event)
	{
		if(Workspace::GetSharedInstance()->GetActiveTool() == Workspace::Tool::Gizmo)
		{
			Workspace::GetSharedInstance()->GetGizmo()->EndMove();
		}
		
		MouseMoved(event);
	}
	
	
	// -----------------------
	// MARK: -
	// MARK: Drag & Drop
	// -----------------------
	
	bool Viewport::AcceptsDropOfObject(RN::Object *object)
	{
		return (object->IsKindOfClass(RN::Model::GetMetaClass()) || object->IsKindOfClass(RN::Value::GetMetaClass()));
	}
	
	void Viewport::HandleDropOfObject(RN::Object *object, const RN::Vector2 &position)
	{
		Workspace *workspace = Workspace::GetSharedInstance();
		Gizmo *gizmo = workspace->GetGizmo();
		
		uint8 group = gizmo->GetCollisionGroup();
		gizmo->SetCollisionGroup(1);
		
		RN::Vector3 rayDirection = GetDirectionForPoint(position*_resolutionFactor);
		RN::Vector3 rayPosition = GetPositionForPoint(position*_resolutionFactor, _camera->GetClipNear());
		RN::Hit hit = RN::World::GetActiveWorld()->GetSceneManager()->CastRay(rayPosition, rayDirection, 1);
		float distance = hit.node ? hit.distance : 15.0f;
		RN::Vector3 targetPosition = rayPosition + rayDirection * distance;
		
		gizmo->SetCollisionGroup(group);
		WorldAttachment::GetSharedInstance()->RequestSceneNode(object, targetPosition);
	}
}
