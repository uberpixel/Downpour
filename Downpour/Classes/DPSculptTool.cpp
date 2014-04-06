//
//  DPSculptTool.cpp
//  Downpour
//
//  Created by Nils Daumann on 05.04.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
//

#include "DPSculptTool.h"

namespace DP
{
	RNDefineMeta(SculptTool, RN::Entity)
	
	SculptTool::SculptTool(Viewport *viewport) :
		_viewport(viewport),
		_mode(Mode::Add),
		_checkLastPosition(false)
	{
		SetModel(RN::Model::WithFile(RN::PathManager::Join(Workspace::GetSharedInstance()->GetResourcePath(), "sculpting_sphere.sgm")));
		
		GetModel()->GetMaterialAtIndex(0, 0)->SetLighting(false);
		GetModel()->GetMaterialAtIndex(0, 0)->SetDiffuseColor(RN::Color(0.1f, 1.0f, 0.2f, 0.7f));
		
		SetRenderGroup(31);
		SetCollisionGroup(1);
		SetFlags(GetFlags() | RN::SceneNode::Flags::HideInEditor | RN::SceneNode::Flags::NoSave);
		
		SetScale(RN::Vector3(3.0f));
		
		_previousTool = Workspace::GetSharedInstance()->GetActiveTool();
		Workspace::GetSharedInstance()->SetActiveTool(DP::Workspace::Tool::Sculpting);
	}
	
	SculptTool::~SculptTool()
	{
		Workspace::GetSharedInstance()->SetActiveTool(_previousTool);
	}
	
	void SculptTool::SetTarget(RN::Sculptable *target)
	{
		_target = target;
	}
	
	void SculptTool::SetMode(Mode mode)
	{
		_mode = mode;
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
			SetWorldPosition(hit.position);
			
			if(input->IsMousePressed(0))
			{
				if(!_checkLastPosition || _lastPosition.GetDistance(GetWorldPosition()) > GetWorldScale().GetMax()*1.01f)
				{
					if(_mode == Mode::Add)
					{
						_lastPosition = GetWorldPosition();
						_target->SetSphere(GetWorldPosition(), GetWorldScale().GetMax());
					}
					else if(_mode == Mode::Substract)
					{
						_lastPosition = GetWorldPosition();
						_target->RemoveSphere(GetWorldPosition(), GetWorldScale().GetMax());
					}
					
					_checkLastPosition = true;
				}
			}
			else
			{
				_checkLastPosition = false;
			}
		}
	}
}