//
//  DPSculptTool.cpp
//  Downpour
//
//  Created by Nils Daumann on 05.04.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
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
		_radius("radius", 3.0f, &SculptTool::GetRadius, &SculptTool::SetRadius)
	{
		AddObservable(&_radius);
		
		RN::Model *model = RN::Model::WithFile(RN::PathManager::Join(Workspace::GetSharedInstance()->GetResourcePath(), "sculpting_sphere.sgm"));
		model->GetMaterialAtIndex(0, 0)->SetLighting(false);
		model->GetMaterialAtIndex(0, 0)->SetDiffuseColor(RN::Color(0.1f, 2.0f, 0.2f, 0.7f));
		SetModel(model);
		
		SetRenderGroup(31);
		SetCollisionGroup(1);
		SetFlags(GetFlags() | RN::SceneNode::Flags::HideInEditor | RN::SceneNode::Flags::NoSave);
		
		SetScale(RN::Vector3(3.0f));
	}
	
	SculptTool::~SculptTool()
	{
	}
	
	void SculptTool::SetTarget(RN::Sculptable *target)
	{
		_target = target;
	}
	
	void SculptTool::SetMode(Mode mode)
	{
		_mode = mode;
	}
	
	void SculptTool::SetRadius(float radius)
	{
		_radius = radius;
		SetScale(RN::Vector3(radius));
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
			if(_mode == Mode::Add)
			{
				_target->SetSphere(GetWorldPosition(), GetWorldScale().GetMax());
			}
			else if(_mode == Mode::Substract)
			{
				_target->RemoveSphere(GetWorldPosition(), GetWorldScale().GetMax());
			}
		}
	}
}