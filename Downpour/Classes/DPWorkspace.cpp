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

namespace DP
{
	RNDefineSingleton(Workspace)
	
	Workspace::Workspace(RN::Module *module) :
		RN::UI::Widget(RN::UI::Widget::StyleBorderless),
		_module(module)
	{
		MakeShared();
		SetWidgetLevel(kRNUIWidgetLevelBackground);
		
		// Capture the current state of the scene
		_state = new SavedState();
		
		// File tree
		_fileTree = new FileTree();
		_fileTree->SetFrame(RN::Rect(0.0f, 0.0f, 250.0f, 0.0f));
		_fileTree->SetAutoresizingMask(RN::UI::View::AutoresizingFlexibleHeight);
		
		// Viewport
		_viewport = new Viewport();
		_viewport->SetFrame(RN::Rect(250.0f, 0.0f, 0.0f, 0.0f));
		_viewport->SetAutoresizingMask(RN::UI::View::AutoresizingFlexibleHeight | RN::UI::View::AutoresizingFlexibleWidth);
		
		GetContentView()->AddSubview(_fileTree);
		GetContentView()->AddSubview(_viewport);
		
		MakeFirstResponder(_viewport); // Make the viewport the first responder to allow camera movement
		
		_worldAttachment = new WorldAttachment();
		RN::WorldCoordinator::GetSharedInstance()->GetWorld()->AddAttachment(_worldAttachment);
		
		UpdateSize();
		RN::MessageCenter::GetSharedInstance()->AddObserver(kRNUIServerDidResizeMessage, std::bind(&Workspace::UpdateSize, this), this);
	}
	
	Workspace::~Workspace()
	{
		RN::MessageCenter::GetSharedInstance()->RemoveObserver(this);
		
		_fileTree->Release();
		// Restore the old state
		delete _state;
	}
	
	
	void Workspace::UpdateSize()
	{
		RN::UI::Server *server = RN::UI::Server::GetSharedInstance();
		SetFrame(RN::Rect(0.0f, 0.0f, server->GetWidth(), server->GetHeight()));
	}
}
