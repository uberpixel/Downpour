//
//  DPMain.cpp
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

#include <Rayne/Rayne.h>
#include "DPWorkspace.h"

namespace DP
{
	static bool __activated = false;
	static Workspace *__workspace = nullptr;
	static RN::Module *__module   = nullptr;
	
	void ActivateDownpour()
	{
		__workspace = new Workspace(__module);
		__workspace->Open();
		__workspace->MakeKeyWidget();
	}
	
	void DeactivateDownpour()
	{
		RN::MessageCenter::GetSharedInstance()->PostMessage(RNCSTR("DPClose"), nullptr, nullptr);
		
		__workspace->Close();
		__workspace->Release();
	}
	
	
	
	void ToggleDownpour()
	{
		// Activating/Deactivating downpour requires UI changes which must be done on the main thread
		
		RN::Kernel::GetSharedInstance()->ScheduleFunction([] {
			__activated = !__activated;
			__activated ? ActivateDownpour() : DeactivateDownpour();
		});
	}
}



extern "C" RNMODULEAPI bool RNModuleConstructor(RN::ModuleExports *exports)
{
	exports->version = kRNABIVersion;
	
	// The module loader will bail out on us when the ABI versions don't match
	// But we shouldn't touch anything. RN::GetABIVersion() is guaranteed to stay ABI
	// compatible, so it's safe to use.
	
	if(RN::GetABIVersion() == kRNABIVersion)
	{
		// Register some callbacks to allow toggling downpour on or off and use the Module as cookie
		RN::MessageCenter::GetSharedInstance()->AddObserver(RNCSTR("DPToggle"), std::bind(&DP::ToggleDownpour), exports->module);
		RN::MessageCenter::GetSharedInstance()->AddObserver(kRNInputEventMessage, [](RN::Message *message) {
			
			RN::Event *event = static_cast<RN::Event *>(message);
			
			if(event->GetType() == RN::Event::Type::KeyDown && event->GetCode() == RN::KeyF11)
			{
				DP::ToggleDownpour();
			}
			
		}, exports->module);
		
		// Add the custom UI style
		std::string path = RN::PathManager::Join(exports->module->GetPath(), "Resources/uistyle.json");
		RN::UI::Style::GetSharedInstance()->LoadStyle(path, RNCSTR("downpour"));
		
		DP::__module = exports->module;
	}
	
	return true;
}

extern "C" RNMODULEAPI void RNModuleDestructor()
{
	// There is no guarantee that the destructor is invoked with an autorelease pool in place
	RN::AutoreleasePool pool;
	
	if(DP::__activated)
		DP::DeactivateDownpour();
	
	RN::MessageCenter::GetSharedInstance()->RemoveObserver(DP::__module);
}
