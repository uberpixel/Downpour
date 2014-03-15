//
//  DPGizmo.cpp
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

#include "DPGizmo.h"
#include "DPWorkspace.h"

namespace DP
{
	RNDefineMeta(Gizmo, RN::Entity)
	
	Gizmo::Gizmo(RN::Camera *camera) :
		_camera(camera),
		_selection(nullptr),
		_mode(Mode::Move),
		_active(false)
	{
		std::string gizmo = RN::PathManager::Join(Workspace::GetSharedInstance()->GetResourcePath(), "gizmo.sgm");
		RN::Model *model = RN::Model::WithFile(gizmo);
		
		for(size_t i = 0; i < model->GetMeshCount(0); i ++)
			model->GetMaterialAtIndex(0, i)->SetLighting(false);
		
		SetModel(model);
		SetRenderGroup(31);
		SetSelection(nullptr);
		SetFlags(GetFlags() | RN::SceneNode::Flags::HideInEditor);
		
		RN::MessageCenter::GetSharedInstance()->AddObserver(kDPWorkspaceSelectionChanged, [this](RN::Message *message) {
			SetSelection(static_cast<RN::Array *>(message->GetObject()));
		}, this);
	}
	
	Gizmo::~Gizmo()
	{
		RN::MessageCenter::GetSharedInstance()->RemoveObserver(this);
	}
	
	void Gizmo::SetMode(Mode mode)
	{
		_mode = mode;
	}
	
	void Gizmo::SetSelection(RN::Array *selection)
	{
		_selection = selection;
		
		RN::SceneNode::Flags flags = GetFlags() & ~RN::SceneNode::Flags::Hidden;
		if(!_selection)
		{
			SetCollisionGroup(1);
			flags |= RN::SceneNode::Flags::Hidden;
		}
		else
		{
			SetCollisionGroup(0);
		}
		
		SetFlags(flags);
	}
	
	void Gizmo::UpdateEditMode(float delta)
	{
		RN::Entity::UpdateEditMode(delta);
		
		if(_selection)
		{
			RN::Vector3 center;
			
			_selection->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop) {
				center += node->GetWorldPosition();
			});
			
			SetPosition(center / _selection->GetCount());
			SetScale(RN::Vector3((_camera->GetPosition()-GetPosition()).GetLength()) * 0.08f);
		}
	}
	
	
	
	RN::Vector3 Gizmo::CameraToWorld(const RN::Vector3 &dir)
	{
		RN::Vector3 ndcPos(dir.x, dir.y, dir.z * 2.0f - 1.0f);
		RN::Vector4 clipPos;
		const RN::Matrix &projection = _camera->GetProjectionMatrix();
		
		clipPos.w = projection.m[14] / (ndcPos.z + projection.m[10]);
		clipPos = RN::Vector4(ndcPos*clipPos.w, clipPos.w);
		
		RN::Vector4 temp = _camera->GetInverseProjectionMatrix() * clipPos;
		temp = _camera->GetInverseViewMatrix() * temp;
		
		return RN::Vector3(temp);
	}
	
	void Gizmo::BeginMove(uint32 selection, const RN::Vector2 &mousePos)
	{
		_active = true;
		_selectedMesh = selection;
		_previousMouse = mousePos;
		
		RN_ASSERT(selection <= 3, "Invalid mesh selection!");
	}
	
	void Gizmo::ContinueMove(const RN::Vector2 &mousePos)
	{
		RN_ASSERT(_active, "Gizmo needs to be activated before moving!");
		RN_ASSERT(_selection, "Gizmo needs a selection to move!");
		
		RN::Vector3 direction;
		switch(_selectedMesh)
		{
			case 0:
				direction = RN::Vector3(1.0f, 0.0f, 0.0f);
				break;
			case 1:
				direction = RN::Vector3(0.0f, 1.0f, 0.0f);
				break;
			case 2:
				direction = RN::Vector3(0.0f, 0.0f, 1.0f);
				break;
			case 3:
				direction = RN::Vector3(1.0f, 1.0f, 1.0f);
				break;
		}
		
		float dist = (_camera->GetPosition()-GetPosition()).GetLength();
		RN::Vector2 temp = mousePos;
		
		temp /= _camera->GetFrame().Size();
		temp.y = 1.0f - temp.y;
		temp *= 2.0f;
		temp -= 1.0f;
		
		RN::Vector3 delta = _camera->ToWorld(RN::Vector3(temp, dist));
		
		temp = _previousMouse;
		temp /= _camera->GetFrame().Size();
		temp.y = 1.0f - temp.y;
		temp *= 2.0f;
		temp -= 1.0f;
		
		delta -= _camera->ToWorld(RN::Vector3(temp, dist));
		
		delta *= direction;
		
		
		switch(_mode)
		{
			case Mode::Move:
			{
				_selection->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop) {
					node->Translate(direction * delta);
				});
				
				break;
			}
				
			case Mode::Scale:
			{
				_selection->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop) {
					(_selectedMesh == 3) ? node->Scale(direction * delta.GetLength() * 0.1f * (delta.x < 0.0f ? - 1.0f : 1.0f)) : node->Scale(direction * delta * 0.1f);
				});
				
				break;
			}
				
			case Mode::Rotate:
			{
				_selection->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop) {
					node->Rotate(direction * delta);
				});
				
				break;
			}
		}
		
		_previousMouse = mousePos;
	}
	
	void Gizmo::EndMove()
	{
		_active = false;
	}
}
