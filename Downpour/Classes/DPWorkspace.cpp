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
#include "DPInfoPanel.h"
#include "DPIPPanel.h"

#define kDPWorkspaceToolbarHeight 40.0f

namespace DP
{
	RNDefineSingleton(Workspace)
	
	Workspace::Workspace(RN::Module *module) :
		RN::UI::Widget(RN::UI::Widget::Style::Borderless, RN::Rect(0.0f, 0.0f, 1024.0f, 768.0f)),
		_module(module),
		_selection(nullptr),
		_pasteBoard(nullptr)
	{
		MakeShared();
		SetWidgetLevel(kRNUIWidgetLevelBackground);
		
		// Capture the current state of the scene
		_state = new SavedState();
		
		// File tree
		_fileTree = new WidgetContainer<FileTree>(RNCSTR("Project"));
		_fileTree->SetFrame(RN::Rect(0.0f, kDPWorkspaceToolbarHeight, 230.0f, 512.0f - kDPWorkspaceToolbarHeight));
		_fileTree->SetAutoresizingMask(RN::UI::View::AutoresizingMask::FlexibleHeight | RN::UI::View::AutoresizingMask::FlexibleRightMargin | RN::UI::View::AutoresizingMask::FlexibleBottomMargin);
		
		// Node picker
		_nodePicker = new WidgetContainer<NodeClassPicker>(RNCSTR("Node Class"));
		_nodePicker->SetFrame(RN::Rect(0.0f, 512.0f, 230.0f, 256.0f));
		_nodePicker->SetAutoresizingMask(RN::UI::View::AutoresizingMask::FlexibleHeight | RN::UI::View::AutoresizingMask::FlexibleRightMargin | RN::UI::View::AutoresizingMask::FlexibleTopMargin);
		
		// Viewport
		_viewport = new WidgetContainer<Viewport>(RNCSTR("Scene"));
		_viewport->SetFrame(RN::Rect(230.0f, kDPWorkspaceToolbarHeight, 534.0f, 768.0f - kDPWorkspaceToolbarHeight));
		_viewport->SetAutoresizingMask(RN::UI::View::AutoresizingMask::FlexibleHeight | RN::UI::View::AutoresizingMask::FlexibleWidth);
		
		// Inspector views
		_inspectors = new WidgetContainer<InspectorViewContainer>(RNCSTR("Inspector"));
		_inspectors->SetFrame(RN::Rect(764.0f, kDPWorkspaceToolbarHeight, 260.0f, 512.0f - kDPWorkspaceToolbarHeight));
		_inspectors->SetAutoresizingMask(RN::UI::View::AutoresizingMask::FlexibleHeight | RN::UI::View::AutoresizingMask::FlexibleLeftMargin | RN::UI::View::AutoresizingMask::FlexibleBottomMargin);
		
		// Hierarchy
		_hierarchy = new WidgetContainer<SceneHierarchy>(RNCSTR("Hierarchy"));
		_hierarchy->SetFrame(RN::Rect(764.0f, 512.0f, 260.0f, 256.0f));
		_hierarchy->SetAutoresizingMask(RN::UI::View::AutoresizingMask::FlexibleHeight | RN::UI::View::AutoresizingMask::FlexibleLeftMargin | RN::UI::View::AutoresizingMask::FlexibleTopMargin);
		
		// Toolbar
		CreateToolbar();
		
		GetContentView()->AddSubview(_fileTree);
		GetContentView()->AddSubview(_nodePicker);
		GetContentView()->AddSubview(_viewport);
		GetContentView()->AddSubview(_inspectors);
		GetContentView()->AddSubview(_hierarchy);
		
		
		MakeFirstResponder(_viewport); // Make the viewport the first responder to allow camera movement
		
		_worldAttachment = WorldAttachment::GetSharedInstance();
		_worldAttachment->Activate(_viewport->GetContent()->GetEditorCamera());
		RN::WorldCoordinator::GetSharedInstance()->GetWorld()->AddAttachment(_worldAttachment);
		
		_gizmo = new Gizmo(_viewport->GetContent()->GetCamera());
		
		CreateMainMenu();
		UpdateSize();
		
		RN::MessageCenter::GetSharedInstance()->AddObserver(kRNUIServerDidResizeMessage, std::bind(&Workspace::UpdateSize, this), this);
	}
	
	Workspace::~Workspace()
	{
		RN::MessageCenter::GetSharedInstance()->RemoveObserver(this);
		RN::WorldCoordinator::GetSharedInstance()->GetWorld()->RemoveAttachment(_worldAttachment);
		
		_viewport->Release();
		_fileTree->Release();
		_nodePicker->Release();
		_inspectors->Release();
		_hierarchy->Release();
		
		_gizmo->RemoveFromWorld();
		_gizmo->Release();
		
		// Restore the old state
		delete _state;
		
		ResignShared();
	}
	
	void Workspace::CreateToolbar()
	{
		_toolbar = new RN::UI::View();
		_toolbar->SetFrame(RN::Rect(0.0f, 0.0f, 1024.0f, kDPWorkspaceToolbarHeight));
		_toolbar->SetAutoresizingMask(RN::UI::View::AutoresizingMask::FlexibleWidth);
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
		
		
		_gizmoSpace = new RN::UI::Button(RN::UI::Button::Type::Bezel);
		_gizmoSpace->SetFrame(RN::Rect(210.0f, 5.0f, 80.0f, 30.0f));
		_gizmoSpace->SetBehavior(RN::UI::Button::Behavior::Switch);
		_gizmoSpace->SetTitleForState(RNCSTR("Local"), RN::UI::Control::State::Normal);
		_gizmoSpace->SetTitleForState(RNCSTR("Global"), RN::UI::Control::State::Selected);
		_gizmoSpace->SetTitleColorForState(ColorScheme::GetUIColor(ColorScheme::Type::FileTree_Text), RN::UI::Control::State::Normal);
		_gizmoSpace->SetFontForState(RN::UI::Style::GetSharedInstance()->GetFont(RN::UI::Style::FontStyle::DefaultFontBold), RN::UI::Control::State::Normal);
		_gizmoSpace->AddListener(RN::UI::Control::EventType::MouseUpInside, [this](RN::UI::Control *control, RN::UI::Control::EventType event) {
			
			Gizmo::Space space = (control->IsSelected()) ? Gizmo::Space::Global : Gizmo::Space::Local;
			_gizmo->SetSpace(space);
			
		}, this);
		
		_toolbar->AddSubview(_gizmoTool->Autorelease());
		_toolbar->AddSubview(_gizmoSpace->Autorelease());
		
		GetContentView()->AddSubview(_toolbar->Autorelease());
	}
	
	void Workspace::CreateMainMenu()
	{
		RN::UI::Menu *menu = new RN::UI::Menu();
		
		// File menu
		RN::UI::Menu *fileMenu = new RN::UI::Menu();
		RN::UI::MenuItem *fileItem = RN::UI::MenuItem::WithTitle(RNCSTR("File"));
		fileItem->SetSubMenu(fileMenu->Autorelease());
		
		fileMenu->AddItem(RN::UI::MenuItem::WithTitle(RNCSTR("New Level")));
		fileMenu->AddItem(RN::UI::MenuItem::WithTitleAndKeyEquivalent(RNCSTR("Open Level..."), std::bind(&Workspace::OpenLevel, this), RNCSTR("o")));
		fileMenu->AddItem(RN::UI::MenuItem::SeparatorItem());
		fileMenu->AddItem(RN::UI::MenuItem::WithTitleAndKeyEquivalent(RNCSTR("Save Level"), std::bind(&Workspace::Save, this), RNCSTR("s")));
		fileMenu->AddItem([&]() -> RN::UI::MenuItem *{
			
			RN::UI::MenuItem *item = RN::UI::MenuItem::WithTitleAndKeyEquivalent(RNCSTR("Save Level as.."), std::bind(&Workspace::SaveAs, this), RNCSTR("s"));
			item->SetKeyEquivalentModifierMask(RN::KeyModifier::KeyAction | RN::KeyModifier::KeyShift);
			
			return item;
			
		}());
		
		// Edit menu
		RN::UI::Menu *editMenu = new RN::UI::Menu();
		RN::UI::MenuItem *editItem = RN::UI::MenuItem::WithTitle(RNCSTR("Edit"));
		editItem->SetSubMenu(editMenu->Autorelease());
		
		editMenu->AddItem(RN::UI::MenuItem::WithTitleAndKeyEquivalent(RNCSTR("Cut"), std::bind(&Workspace::Cut, this), RNCSTR("x")));
		editMenu->AddItem(RN::UI::MenuItem::WithTitleAndKeyEquivalent(RNCSTR("Copy"), std::bind(&Workspace::Copy, this), RNCSTR("c")));
		editMenu->AddItem(RN::UI::MenuItem::WithTitleAndKeyEquivalent(RNCSTR("Paste"), std::bind(&Workspace::Paste, this), RNCSTR("v")));
		editMenu->AddItem(RN::UI::MenuItem::SeparatorItem());
		editMenu->AddItem(RN::UI::MenuItem::WithTitleAndKeyEquivalent(RNCSTR("Duplicate"), std::bind(&Workspace::Duplicate, this), RNCSTR("d")));
		editMenu->AddItem(RN::UI::MenuItem::WithTitleAndKeyEquivalent(RNCSTR("Delete"), std::bind(&Workspace::Delete, this), RNCSTR("\b")));
		
		// Scene node menu
		RN::UI::Menu *nodeMenu = new RN::UI::Menu();
		RN::UI::MenuItem *nodeItem = RN::UI::MenuItem::WithTitle(RNCSTR("Scene Node"));
		nodeItem->SetSubMenu(nodeMenu->Autorelease());
		
		nodeMenu->AddItem(RN::UI::MenuItem::WithTitle(RNCSTR("Create Scene Node")));
		nodeMenu->AddItem([&]() -> RN::UI::MenuItem * {
			
			RN::UI::Menu *createMenu = new RN::UI::Menu();
			RN::UI::MenuItem *createItem = RN::UI::MenuItem::WithTitle(RNCSTR("Create..."));
			createItem->SetSubMenu(createMenu->Autorelease());
			
			createMenu->AddItem(RN::UI::MenuItem::WithTitle(RNCSTR("Entity")));
			createMenu->AddItem(RN::UI::MenuItem::WithTitle(RNCSTR("Billboard")));
			createMenu->AddItem(RN::UI::MenuItem::WithTitle(RNCSTR("Decal")));
			createMenu->AddItem(RN::UI::MenuItem::SeparatorItem());
			createMenu->AddItem(RN::UI::MenuItem::WithTitle(RNCSTR("Point Light")));
			createMenu->AddItem(RN::UI::MenuItem::WithTitle(RNCSTR("Spot Light")));
			createMenu->AddItem(RN::UI::MenuItem::WithTitle(RNCSTR("Directional Light")));
			
			return createItem;
			
		}());
		
		// Network menu
		RN::UI::Menu *networkMenu = new RN::UI::Menu();
		RN::UI::MenuItem *networkItem = RN::UI::MenuItem::WithTitle(RNCSTR("Network"));
		networkItem->SetSubMenu(networkMenu->Autorelease());
		
		networkMenu->AddItem(RN::UI::MenuItem::WithTitle(RNCSTR("Host Session"), std::bind(&Workspace::HostSession, this)));
		networkMenu->AddItem(RN::UI::MenuItem::WithTitle(RNCSTR("Connect to Session"), std::bind(&Workspace::ConnectToSession, this)));
		networkMenu->AddItem(RN::UI::MenuItem::WithTitle(RNCSTR("Disconnect from Session"), std::bind(&Workspace::DisconnectFromSession, this)));
		
		menu->AddItem(fileItem);
		menu->AddItem(editItem);
		menu->AddItem(nodeItem);
		menu->AddItem(networkItem);
		
		RN::UI::Server::GetSharedInstance()->SetMainMenu(menu->Autorelease());
	}
	
	
	// -----------------------
	// MARK: -
	// MARK: Responder stuff
	// -----------------------
	
	extern void ActivateDownpour();
	extern void DeactivateDownpour();
	
	static const char *__DPCookie = "__DPCookie";
	
	void Workspace::OpenLevel()
	{
		RN::OpenPanel *panel = new RN::OpenPanel();
		panel->SetTitle("Open Level");
		panel->SetCanCreateDirectories(true);
		
		panel->Show([&](bool result, const std::vector<std::string> &paths) {
			
			if(result)
			{
				std::string path = paths.front();
				
				RN::Kernel::GetSharedInstance()->ScheduleFunction([path]() {
					
					{
						RN::AutoreleasePool pool;
						DeactivateDownpour();
					}
					
					RN::Kernel::GetSharedInstance()->ScheduleFunction([path]() {
					
						RN::MessageCenter::GetSharedInstance()->AddObserver(kRNWorldCoordinatorDidFinishLoadingMessage, [](RN::Message *message) {
							
							ActivateDownpour();
							RN::MessageCenter::GetSharedInstance()->RemoveObserver(const_cast<char *>(__DPCookie));
							
						}, const_cast<char *>(__DPCookie));
						
						RN::WorldCoordinator::GetSharedInstance()->LoadWorld(path);
						
					});
				});
			}
		});
		
		panel->Release();
	}
	
	void Workspace::Save()
	{
		std::string path = RN::WorldCoordinator::GetSharedInstance()->GetWorldFile();
		if(path.empty())
		{
			SaveAs();
			return;
		}
		
		try
		{
			RN::WorldCoordinator::GetSharedInstance()->SaveWorld(path);
			RNInfo("Downpour: Saved world succesfully to %s", path.c_str());
		}
		catch(RN::Exception e)
		{
			ExceptionPanel *exceptionPanel = new ExceptionPanel(e);
			exceptionPanel->Open();
			exceptionPanel->Release();
		}
	}
	
	void Workspace::SaveAs()
	{
		RN::SavePanel *panel = new RN::SavePanel();
		panel->SetTitle("Save as...");
		panel->SetCanCreateDirectories(true);
		
		panel->Show([&](bool result, const std::string &path) {
			
			if(result)
			{
				try
				{
					RN::WorldCoordinator::GetSharedInstance()->SaveWorld(path);
					RNInfo("Downpour: Saved world succesfully to %s", path.c_str());
				}
				catch(RN::Exception e)
				{
					ExceptionPanel *exceptionPanel = new ExceptionPanel(e);
					exceptionPanel->Open();
					exceptionPanel->Release();
				}
			}
			
		});
		
		panel->Release();
	}
	
	
	void Workspace::Duplicate()
	{
		RN::Array *selection = GetSelection();
		if(selection)
		{
			_worldAttachment->DuplicateSceneNodes(selection);
		}
	}
	
	void Workspace::Delete()
	{
		RN::Array *selection = GetSelection();
		if(selection)
		{
			_worldAttachment->DeleteSceneNodes(selection);
		}
		
		SetSelection(nullptr);
	}
	
	void Workspace::Copy()
	{
		RN::SafeRelease(_pasteBoard);
		
		if(_selection)
			_pasteBoard = _selection->Copy();
	}
	
	void Workspace::Paste()
	{
		WorldAttachment::GetSharedInstance()->DuplicateSceneNodes(_pasteBoard);
	}
	
	void Workspace::Cut()
	{
		Copy();
		Delete();
	}
	
	void Workspace::HostSession()
	{
		_worldAttachment->CreateServer();
	}
	
	void Workspace::ConnectToSession()
	{
		IPPanel *panel = new IPPanel([this](std::string ip) {
			
			_worldAttachment->CreateClient();
			_worldAttachment->Connect(ip);
			
		});
		
		panel->Open();
		panel->Release();
	}
	
	void Workspace::DisconnectFromSession()
	{
		_worldAttachment->DestroyHost();
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
		
		
		RN::Array *sanitized = (new RN::Array(_selection->GetCount()))->Autorelease();
		
		_selection->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop) {
			
			if(node->IsKindOfClass(EditorIcon::MetaClass()))
			{
				EditorIcon *icon = static_cast<EditorIcon *>(node);
				sanitized->AddObject(icon->GetSceneNode());
				
				return;
			}
			
			if(node->GetFlags() & RN::SceneNode::Flags::LockedInEditor)
				return;
			
			sanitized->AddObject(node);
			
		});
		
		
		_selection->Release();
		_selection = (sanitized->GetCount() > 0) ? sanitized->Retain() : nullptr;
		
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
		if(RN::Input::GetSharedInstance()->GetModifierKeys() & kDPWorkspaceActionKey)
		{
			if(!_selection)
				_selection = new RN::Array();
			
			if(!_selection->ContainsObject(selection))
				_selection->AddObject(selection);
			else
				_selection->RemoveObject(selection);
		}
		else
		{
			RN::SafeRelease(_selection);
			_selection = RN::Array::WithObjects(selection, nullptr)->Retain();
		}
		
		SanitizeAndPostSelection();
	}
	
	void Workspace::SetSelection(std::nullptr_t null)
	{
		RN::SafeRelease(_selection);
		SanitizeAndPostSelection();
	}
	
	// -----------------------
	// MARK: -
	// MARK: Event handling
	// -----------------------
	
	void Workspace::KeyDown(RN::Event *event)
	{
		switch(event->GetCode())
		{
			case RN::KeyESC:
			{
				if(_selection)
				{
					SetSelection(nullptr);
					return;
				}
				
				RN::MessageCenter::GetSharedInstance()->PostMessage(RNCSTR("DPToggle"), nullptr, nullptr);
				break;
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
