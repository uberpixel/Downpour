//
//  DPWorkspace.cpp
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

#include "DPWorkspace.h"
#include "DPEditorIcon.h"

#define kDPWorkspaceToolbarHeight 40.0f

namespace DP
{
	RNDefineSingleton(Workspace)
	
	Workspace::Workspace(RN::Module *module) :
		RN::UI::Widget(RN::UI::Widget::StyleBorderless, RN::Rect(0.0f, 0.0f, 1024.0f, 768.0f)),
		_module(module),
		_selection(nullptr)
	{
		MakeShared();
		SetWidgetLevel(kRNUIWidgetLevelBackground);
		
		// Capture the current state of the scene
		_state = new SavedState();
		
		// File tree
		_fileTree = new WidgetContainer<FileTree>(RNCSTR("Project"));
		_fileTree->SetFrame(RN::Rect(0.0f, kDPWorkspaceToolbarHeight, 230.0f, 768.0f - kDPWorkspaceToolbarHeight));
		_fileTree->SetAutoresizingMask(RN::UI::View::AutoresizingFlexibleHeight);
		
		// Viewport
		_viewport = new WidgetContainer<Viewport>(RNCSTR("Scene"));
		_viewport->SetFrame(RN::Rect(230.0f, kDPWorkspaceToolbarHeight, 534.0f, 768.0f - kDPWorkspaceToolbarHeight));
		_viewport->SetAutoresizingMask(RN::UI::View::AutoresizingFlexibleHeight | RN::UI::View::AutoresizingFlexibleWidth);
		
		// Inspector views
		_inspectors = new WidgetContainer<InspectorViewContainer>(RNCSTR("Inspector"));
		_inspectors->SetFrame(RN::Rect(764.0f, kDPWorkspaceToolbarHeight, 260.0f, 512.0f - kDPWorkspaceToolbarHeight));
		_inspectors->SetAutoresizingMask(RN::UI::View::AutoresizingFlexibleHeight | RN::UI::View::AutoresizingFlexibleLeftMargin | RN::UI::View::AutoresizingFlexibleBottomMargin);
		
		// Hierarchy
		_hierarchy = new WidgetContainer<SceneHierarchy>(RNCSTR("Hierarchy"));
		_hierarchy->SetFrame(RN::Rect(764.0f, 512.0f, 260.0f, 256.0f));
		_hierarchy->SetAutoresizingMask(RN::UI::View::AutoresizingFlexibleHeight | RN::UI::View::AutoresizingFlexibleLeftMargin | RN::UI::View::AutoresizingFlexibleTopMargin);
		
		// Toolbar
		CreateToolbar();
		
		GetContentView()->AddSubview(_fileTree);
		GetContentView()->AddSubview(_viewport);
		GetContentView()->AddSubview(_inspectors);
		GetContentView()->AddSubview(_hierarchy);
		
		
		MakeFirstResponder(_viewport); // Make the viewport the first responder to allow camera movement
		
		_worldAttachment = new WorldAttachment(_viewport->GetContent()->GetEditorCamera());
		RN::WorldCoordinator::GetSharedInstance()->GetWorld()->AddAttachment(_worldAttachment);
		
		_gizmo = new Gizmo(_viewport->GetContent()->GetCamera());
		
		UpdateSize();
		RN::MessageCenter::GetSharedInstance()->AddObserver(kRNUIServerDidResizeMessage, std::bind(&Workspace::UpdateSize, this), this);
	}
	
	Workspace::~Workspace()
	{
		RN::MessageCenter::GetSharedInstance()->RemoveObserver(this);
		RN::WorldCoordinator::GetSharedInstance()->GetWorld()->RemoveAttachment(_worldAttachment);
		
		_viewport->Release();
		_fileTree->Release();
		_inspectors->Release();
		_hierarchy->Release();
		
		_gizmo->Release();
		_worldAttachment->Release();
		
		// Restore the old state
		delete _state;
	}
	
	void Workspace::CreateToolbar()
	{
		_toolbar = new RN::UI::View();
		_toolbar->SetFrame(RN::Rect(0.0f, 0.0f, 1024.0f, kDPWorkspaceToolbarHeight));
		_toolbar->SetAutoresizingMask(RN::UI::View::AutoresizingFlexibleWidth);
		_toolbar->SetBackgroundColor(ColorScheme::GetColor(ColorScheme::Type::Background_Light));
		
		_gizmoTool = new RN::UI::SegmentView();
		_gizmoTool->SetFrame(RN::Rect(50.0f, 5.0f, 150.0f, 30.0f));
		_gizmoTool->InsertegmentAtIndex(RN::UI::Image::WithFile(RN::PathManager::Join(GetResourcePath(), "icons/move.png")), 0);
		_gizmoTool->InsertegmentAtIndex(RN::UI::Image::WithFile(RN::PathManager::Join(GetResourcePath(), "icons/scale.png")), 1);
		_gizmoTool->InsertegmentAtIndex(RN::UI::Image::WithFile(RN::PathManager::Join(GetResourcePath(), "icons/rotate.png")), 2);
		_gizmoTool->SetSegmentAtIndexEnabled(0, true);
		_gizmoTool->AddListener(RN::UI::Control::EventType::ValueChanged, [this](RN::UI::Control *control, RN::UI::Control::EventType event) {
			
			// This is a bit of an ugly hack to allow for single selection only
			// Basically the state of the toolbar and the gizmo are converted into bitfields which are then XOR'd
			// to retrieve the newly selected tool.
			
			uint32 selection = 0;
			
			for(int i = 0; i < 3; i ++)
			{
				selection |= (_gizmoTool->IsSegmentAtIndexEnabled(i)) ? (1 << i) : 0;
				_gizmoTool->SetSegmentAtIndexEnabled(i, false);
			}
			
			selection = (1 << static_cast<uint32>(_gizmo->GetMode())) ^ selection;
			selection = selection >> 1;
			
			_gizmo->SetMode(static_cast<Gizmo::Mode>(selection));
			_gizmoTool->SetSegmentAtIndexEnabled(selection, true);
			
		}, this);
		
		_toolbar->AddSubview(_gizmoTool);
		
		GetContentView()->AddSubview(_toolbar);
	}
	
	
	// -----------------------
	// MARK: -
	// MARK: Selection
	// -----------------------
	
	void Workspace::SanitizeAndPostSelection()
	{
		if(!_selection)
		{
			RN::MessageCenter::GetSharedInstance()->PostMessage(kDPWorkspaceSelectionChanged, _selection, nullptr);
			return;
		}
		
		if(_selection->GetCount() == 0)
		{
			_selection->Release();
			_selection = nullptr;
			
			RN::MessageCenter::GetSharedInstance()->PostMessage(kDPWorkspaceSelectionChanged, _selection, nullptr);
			return;
		}
		
		RN::Array *sanitized = new RN::Array(_selection->GetCount());
		
		_selection->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop) {
			
			if(node->IsKindOfClass(EditorIcon::MetaClass()))
			{
				EditorIcon *icon = static_cast<EditorIcon *>(node);
				sanitized->AddObject(icon->GetSceneNode());
				
				return;
			}
			
			sanitized->AddObject(node);
			
		});
		
		_selection->Release();
		_selection = sanitized;
		
		RN::MessageCenter::GetSharedInstance()->PostMessage(kDPWorkspaceSelectionChanged, _selection, nullptr);
	}
	
	void Workspace::SetSelection(RN::Array *selection)
	{
		RN::SafeRelease(_selection);
		_selection = selection->Copy();
		
		SanitizeAndPostSelection();
	}
	void Workspace::SetSelection(RN::SceneNode *selection)
	{
		RN::SafeRelease(_selection);
		_selection = RN::Array::WithObjects(selection, nullptr)->Retain();
		
		SanitizeAndPostSelection();
	}
	void Workspace::SetSelection(std::nullptr_t null)
	{
		RN::SafeRelease(_selection);
		SanitizeAndPostSelection();
	}
	
	void Workspace::KeyDown(RN::Event *event)
	{
		switch(event->GetCode())
		{
			case RN::KeyDelete:
			{
				RN::Array *selection = Workspace::GetSharedInstance()->GetSelection();
				if(selection)
				{
					selection->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop) {
						
						if(node->GetParent())
							node->RemoveFromParent();
						
						node->GetWorld()->RemoveSceneNode(node);
						node->Autorelease();
						
					});
				}
				
				SetSelection(nullptr);
			}
		}
	}
	
	// -----------------------
	// MARK: -
	// MARK: Resizing
	// -----------------------
	
	void Workspace::UpdateSize()
	{
		RN::UI::Server *server = RN::UI::Server::GetSharedInstance();
		SetFrame(RN::Rect(0.0f, 0.0f, server->GetWidth(), server->GetHeight()));
	}
}
