//
//  DPSculptTool.cpp
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

#include "DPSculptTool.h"
#include "DPWorkspace.h"

namespace DP
{
	RNDefineMeta(SculptTool, RN::Entity)
	
	SculptTool::SculptTool(Viewport *viewport) :
		_viewport(viewport),
		_mode(Mode::Add),
		_hasValidPosition(false),
		_radius("radius", 3.0f, &SculptTool::GetRadius, &SculptTool::SetRadius),
		_size("size", RN::Vector3(3.0f), &SculptTool::GetSize, &SculptTool::SetSize),
		_shape(Shape::Sphere)
	{
		AddObservables({&_radius, &_size});
		
		std::string shaderpath = RN::PathManager::Join(Workspace::GetSharedInstance()->GetResourcePath(), "shaders");
		
		RN::Shader *shader = new RN::Shader();
		shader->SetShaderForType(RN::PathManager::Join(shaderpath, "SculptTool.vsh"), RN::ShaderType::VertexShader);
		shader->SetShaderForType(RN::PathManager::Join(shaderpath, "SculptTool.fsh"), RN::ShaderType::FragmentShader);
		
		_models[0] = RN::Model::WithFile(RN::PathManager::Join(Workspace::GetSharedInstance()->GetResourcePath(), "sculpting_sphere.sgm"))->Retain();
		_models[0]->GetMaterialAtIndex(0, 0)->SetShader(shader);
		_models[0]->GetMaterialAtIndex(0, 0)->SetLighting(false);
		_models[0]->GetMaterialAtIndex(0, 0)->SetDiffuseColor(RN::Color(0.1f, 2.0f, 0.2f, 0.7f));
		_models[0]->GetMaterialAtIndex(0, 0)->AddTexture(_viewport->GetCamera()->GetStorage()->GetDepthTarget());
		_positionUniform[0] = _models[0]->GetMaterialAtIndex(0, 0)->AddShaderUniform("modelPosition", GetWorldPosition());
		_sizeUniform[0] = _models[0]->GetMaterialAtIndex(0, 0)->AddShaderUniform("sphereRadius", _radius);
		
		_models[1] = RN::Model::WithFile(RN::PathManager::Join(Workspace::GetSharedInstance()->GetResourcePath(), "sculpting_cube.sgm"))->Retain();
		_models[1]->GetMaterialAtIndex(0, 0)->SetShader(shader);
		_models[1]->GetMaterialAtIndex(0, 0)->SetLighting(false);
		_models[1]->GetMaterialAtIndex(0, 0)->SetDiffuseColor(RN::Color(0.1f, 2.0f, 0.2f, 0.7f));
		_models[1]->GetMaterialAtIndex(0, 0)->AddTexture(_viewport->GetCamera()->GetStorage()->GetDepthTarget());
		_models[1]->GetMaterialAtIndex(0, 0)->Define("DP_CUBE");
		_positionUniform[1] = _models[1]->GetMaterialAtIndex(0, 0)->AddShaderUniform("modelPosition", GetWorldPosition());
		_sizeUniform[1] = _models[1]->GetMaterialAtIndex(0, 0)->AddShaderUniform("cubeSize", _size);
		
		SetModel(_models[0]);
		
		SetRenderGroup(31);
		SetCollisionGroup(1);
		SetFlags(GetFlags() | RN::SceneNode::Flags::HideInEditor | RN::SceneNode::Flags::NoSave);
		
		SetScale(RN::Vector3(3.0f));
	}
	
	SculptTool::~SculptTool()
	{
		_models[0]->Release();
		_models[1]->Release();
	}
	
	void SculptTool::SetTarget(RN::Sculptable *target)
	{
		_target = target;
	}
	
	void SculptTool::SetMode(Mode mode)
	{
		_mode = mode;
	}
	
	void SculptTool::SetShape(Shape shape)
	{
		_shape = shape;
		SetModel(_models[_shape]);
		
		switch (_shape)
		{
			case Shape::Sphere:
				SetRadius(_radius);
				break;
				
			case Shape::Cube:
				SetSize(_size);
				break;
		}
	}
	
	void SculptTool::SetRadius(float radius)
	{
		_radius = radius;
		SetScale(RN::Vector3(radius));
		_sizeUniform[0]->SetFloatValue(radius);
	}
	
	void SculptTool::SetSize(const RN::Vector3 &size)
	{
		_size = size;
		SetScale(_size);
		_sizeUniform[1]->SetVector(size);
	}
	
	void SculptTool::UpdateEditMode(float delta)
	{
		RN::Entity::UpdateEditMode(delta);
		
		RN::Input *input = RN::Input::GetSharedInstance();
		
		RN::Vector2 mousePos = input->GetMousePosition();
		RN::Vector3 mouseRayStart = _viewport->GetPositionForMouse(mousePos, _viewport->GetCamera()->GetClipNear());
		RN::Vector3 mouseRayDirection = _viewport->GetDirectionForMouse(mousePos);
		
		RN::Hit hit = _target->CastRay(mouseRayStart, mouseRayDirection);
		if(hit.distance > 0)
		{
			SetFlags(GetFlags() & ~RN::SceneNode::Flags::Hidden);
			SetWorldPosition(hit.position);
			_positionUniform[_shape]->SetVector(hit.position);
			_hasValidPosition = true;
		}
		else
		{
			SetFlags(GetFlags() | RN::SceneNode::Flags::Hidden);
			_hasValidPosition = false;
		}
	}
	
	void SculptTool::UseTool()
	{
		if(_hasValidPosition)
		{
			switch(_shape)
			{
				case Shape::Sphere:
				{
					if(_mode == Mode::Add)
					{
						_target->SetSphere(GetWorldPosition(), GetWorldScale().GetMax());
					}
					else if(_mode == Mode::Substract)
					{
						_target->RemoveSphere(GetWorldPosition(), GetWorldScale().GetMax());
					}
					
					break;
				}
					
				case Shape::Cube:
				{
					if(_mode == Mode::Add)
					{
						_target->SetCube(GetWorldPosition(), GetWorldScale());
					}
					else if(_mode == Mode::Substract)
					{
						_target->RemoveCube(GetWorldPosition(), GetWorldScale());
					}
					
					break;
				}
			}
		}
	}
}