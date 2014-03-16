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
		_mode(Mode::Translate),
		_space(Space::Local),
		_active(false),
		_selectedMesh(-1)
	{
		std::string translationPath = RN::PathManager::Join(Workspace::GetSharedInstance()->GetResourcePath(), "gizmo_trans.sgm");
		std::string scalingPath = RN::PathManager::Join(Workspace::GetSharedInstance()->GetResourcePath(), "gizmo_scal.sgm");
		std::string rotationPath = RN::PathManager::Join(Workspace::GetSharedInstance()->GetResourcePath(), "gizmo_rot.sgm");
		
		_modelTranslation = RN::Model::WithFile(translationPath)->Retain();
		_modelScaling = RN::Model::WithFile(scalingPath)->Retain();
		_modelRotation = RN::Model::WithFile(rotationPath)->Retain();
		
		for(size_t i = 0; i < _modelTranslation->GetMeshCount(0); i ++)
			_modelTranslation->GetMaterialAtIndex(0, i)->SetLighting(false);
		
		for(size_t i = 0; i < _modelScaling->GetMeshCount(0); i ++)
			_modelScaling->GetMaterialAtIndex(0, i)->SetLighting(false);
		
		for(size_t i = 0; i < _modelRotation->GetMeshCount(0); i ++)
			_modelRotation->GetMaterialAtIndex(0, i)->SetLighting(false);
		
		SetModel(_modelTranslation);
		SetRenderGroup(31);
		SetSelection(nullptr);
		SetFlags(GetFlags() | RN::SceneNode::Flags::HideInEditor | RN::SceneNode::Flags::NoSave);
		
		_scaleFactor = RN::Settings::GetSharedInstance()->GetFloatForKey(RNCSTR("DPGizmoScale"), 1.0f);
		
		RN::MessageCenter::GetSharedInstance()->AddObserver(kDPWorkspaceSelectionChanged, [this](RN::Message *message) {
			SetSelection(static_cast<RN::Array *>(message->GetObject()));
		}, this);
	}
	
	Gizmo::~Gizmo()
	{
		_modelTranslation->Release();
		_modelScaling->Release();
		_modelRotation->Release();
		RN::MessageCenter::GetSharedInstance()->RemoveObserver(this);
	}
	
	void Gizmo::SetMode(Mode mode)
	{
		_mode = mode;
		SetHighlight(-1);
		
		switch(_mode)
		{
			case Mode::Translate:
			{
				SetModel(_modelTranslation);
				break;
			}
				
			case Mode::Scale:
			{
				SetModel(_modelScaling);
				break;
			}
				
			case Mode::Rotate:
			{
				SetModel(_modelRotation);
				break;
			}
		}
	}
	
	void Gizmo::SetSpace(Space space)
	{
		_space = space;
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
			SetScale(RN::Vector3((_camera->GetPosition()-GetPosition()).GetLength()) * 0.08f * _scaleFactor);
			
			if((_space == Space::Local && _selection->GetCount() == 1) || _mode == Mode::Scale)
			{
				SetRotation(_selection->GetObjectAtIndex<SceneNode>(0)->GetWorldRotation());
			}
			else
			{
				SetRotation(RN::Quaternion());
			}
		}
	}
	
	void Gizmo::SetHighlight(uint32 selection, float factor)
	{
		if(_selectedMesh != selection)
		{
			if(_selectedMesh != -1)
			{
				RN::Material *mat = GetModel()->GetMaterialAtIndex(0, _selectedMesh);
				mat->SetDiffuseColor(_highlightOldColor);
			}
			
			if(selection != -1)
			{
				RN::Material *mat = GetModel()->GetMaterialAtIndex(0, selection);
				_highlightOldColor = mat->GetDiffuseColor();
				RN::Color newcolor = _highlightOldColor * factor;
				newcolor.a = _highlightOldColor.a;
				mat->SetDiffuseColor(newcolor);
			}
			
			_selectedMesh = selection;
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
	
	RN::Vector3 Gizmo::GetMousePosition(const RN::Plane &plane, RN::Vector2 mouse)
	{
		mouse /= _camera->GetFrame().Size();
		mouse.y = 1.0 - mouse.y;
		mouse *= 2.0f;
		mouse -= 1.0f;
		RN::Vector3 camPos = _camera->GetWorldPosition();
		RN::Vector3 dir = _camera->ToWorld(RN::Vector3(mouse, 10.0f))-camPos;
		return plane.CastRay(camPos, dir).position;
	}
	
	RN::Vector3 Gizmo::GetMouseMovement(const RN::Plane &plane, RN::Vector2 from, RN::Vector2 to)
	{
		RN::Vector3 diff = GetMousePosition(plane, to);
		diff -= GetMousePosition(plane, from);
		return diff;
	}
	
	void Gizmo::BeginMove(uint32 selection, const RN::Vector2 &mousePos)
	{
		_active = true;
		SetHighlight(-1);
		SetHighlight(selection, 3.0f);
		_previousMouse = mousePos;
	}
	
	void Gizmo::ContinueMove(const RN::Vector2 &mousePos)
	{
		RN_ASSERT(_active, "Gizmo needs to be activated before moving!");
		RN_ASSERT(_selection, "Gizmo needs a selection to move!");
		
		switch(_mode)
		{
			case Mode::Translate:
			{
				DoTranslation(mousePos);
				break;
			}
				
			case Mode::Scale:
			{
				DoScale(mousePos);
				break;
			}
				
			case Mode::Rotate:
			{
				DoRotation(mousePos);
				break;
			}
		}
		
		_previousMouse = mousePos;
	}
	
	void Gizmo::EndMove()
	{
		_active = false;
		SetHighlight(-1);
	}
	
	void Gizmo::DoTranslation(const RN::Vector2 &mousePos)
	{
		RN_ASSERT(_selectedMesh <= 6, "Invalid mesh selection!");
		
		//Initialize for the selected gizmo part
		RN::Vector3 direction;
		RN::Vector3 normal;
		bool singleAxis = false;
		switch(_selectedMesh)
		{
			case 0:
				direction = RN::Vector3(1.0f, 0.0f, 1.0f);
				normal = RN::Vector3(0.0, 1.0, 0.0);
				break;
			case 1:
				direction = RN::Vector3(0.0f, 1.0f, 1.0f);
				normal = RN::Vector3(1.0, 0.0, 0.0);
				break;
			case 2:
				direction = RN::Vector3(1.0f, 1.0f, 0.0f);
				normal = RN::Vector3(0.0, 0.0, 1.0);
				break;
			case 3:
				direction = RN::Vector3(1.0f, 0.0f, 0.0f);
				normal = RN::Vector3(0.0, 1.0, 0.0);
				singleAxis = true;
				break;
			case 4:
				direction = RN::Vector3(0.0f, 1.0f, 0.0f);
				normal = RN::Vector3(1.0, 0.0, 0.0);
				singleAxis = true;
				break;
			case 5:
				direction = RN::Vector3(0.0f, 0.0f, 1.0f);
				normal = RN::Vector3(0.0, 1.0, 0.0);
				singleAxis = true;
				break;
			case 6:
				direction = RN::Vector3(1.0f, 1.0f, 1.0f);
				normal = -_camera->GetForward();
				break;
		}
		
		//Transform to gizmo space
		direction = GetWorldRotation().GetRotatedVector(direction);
		normal = GetWorldRotation().GetRotatedVector(normal);
		
		//Create a plane for the selected gizmo part
		RN::Plane plane = RN::Plane::WithPositionNormal(GetWorldPosition(), normal);
		
		//Project mouse movement on the plane and get the difference in world coordinates
		RN::Vector3 translation = GetMouseMovement(plane, _previousMouse, mousePos);
		
		//Restrict movement to one axis
		if(singleAxis)
		{
			translation = direction * translation.GetDotProduct(direction);
		}
		
		//Apply the translation to all selected scene nodes
		_selection->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop)
		{
			node->Translate(translation);
		});
	}
	
	void Gizmo::DoScale(RN::Vector2 mousePos)
	{
		RN_ASSERT(_selectedMesh <= 6, "Invalid mesh selection!");
		
		RN::Vector3 direction;
		RN::Vector3 normal;
		switch(_selectedMesh)
		{
			case 0:
				direction = RN::Vector3(0.0f, 1.0f, 1.0f);
				normal = RN::Vector3(1.0, 0.0, 0.0);
				break;
			case 1:
				direction = RN::Vector3(1.0f, 1.0f, 0.0f);
				normal = RN::Vector3(0.0, 0.0, 1.0);
				break;
			case 2:
				direction = RN::Vector3(1.0f, 0.0f, 0.0f);
				normal = RN::Vector3(0.0, 1.0, 0.0);
				break;
			case 3:
				direction = RN::Vector3(0.0f, 1.0f, 0.0f);
				normal = RN::Vector3(1.0, 0.0, 0.0);
				break;
			case 4:
				direction = RN::Vector3(1.0f, 1.0f, 1.0f);
				normal = _camera->GetForward();
				break;
			case 5:
				direction = RN::Vector3(0.0f, 0.0f, 1.0f);
				normal = RN::Vector3(0.0, 1.0, 0.0);
				break;
			case 6:
				direction = RN::Vector3(1.0f, 0.0f, 1.0f);
				normal = RN::Vector3(0.0, 1.0, 0.0);
				break;
		}
		
		//Transform to gizmo space
		direction = GetWorldRotation().GetRotatedVector(direction);
		normal = GetWorldRotation().GetRotatedVector(normal);
		
		//Create a plane for the selected gizmo part
		RN::Plane plane = RN::Plane::WithPositionNormal(GetWorldPosition(), normal);
		
		//Project mouse movement on the plane and get the difference in world coordinates
		RN::Vector3 scaling = GetMouseMovement(plane, _previousMouse, mousePos);
		
		//Restrict movement to one uniform value
		scaling = direction * scaling.GetDotProduct(direction);
		
		scaling = GetWorldRotation().GetConjugated().GetRotatedVector(scaling);
		
		_selection->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop)
		{
			node->Scale(scaling*0.3f);
		});
	}
	
	void Gizmo::DoRotation(RN::Vector2 mousePos)
	{
		RN_ASSERT(_selectedMesh <= 3, "Invalid mesh selection!");
		
		RN::Vector3 normal;
		switch(_selectedMesh)
		{
			case 0:
				normal = RN::Vector3(0.0f, 0.0f, 1.0f);
				break;
			case 1:
				normal = RN::Vector3(1.0f, 0.0f, 0.0f);
				break;
			case 2:
				normal = RN::Vector3(0.0f, 1.0f, 0.0f);
				break;
		}
		
		//Transform to gizmo space
		RN::Vector3 rotatedNormal = GetWorldRotation().GetRotatedVector(normal);
		
		//Create a plane for the selected gizmo part
		RN::Plane plane = RN::Plane::WithPositionNormal(GetWorldPosition(), rotatedNormal);
		
		//Project mouse position onto the plane
		RN::Vector3 mouseNew = GetMousePosition(plane, mousePos) - GetWorldPosition();
		RN::Vector3 mouseOld = GetMousePosition(plane, _previousMouse) - GetWorldPosition();
		
		//Create quaternions looking from the gizmo to the mouse positions
		RN::Quaternion rotTo = RN::Quaternion::WithLookAt(mouseNew, rotatedNormal);
		RN::Quaternion rotFrom = RN::Quaternion::WithLookAt(mouseOld, rotatedNormal);
		
		//Claculate the rotations difference
		RN::Quaternion rotDiff = (rotTo/rotFrom);
		rotDiff = RN::Quaternion::WithAxisAngle(RN::Vector4(normal, rotDiff.GetEulerAngle().x));
		
		//Rotate scene nodes
		_selection->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop)
		{
			RN::Vector3 posDiff = node->GetWorldPosition() - GetWorldPosition();
			posDiff = rotDiff.GetRotatedVector(posDiff);
			node->SetWorldPosition(GetWorldPosition() + posDiff);
			
			if(_space == Space::Global)
			{
				RN::Quaternion rotTemp = rotDiff*node->GetWorldRotation();
				node->SetWorldRotation(rotTemp);
			}
			else
			{
				node->Rotate(rotDiff);
			}
		});
	}
}
