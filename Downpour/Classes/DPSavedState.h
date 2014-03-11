//
//  DPSavedState.h
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

#ifndef __DPSAVEDSTATE_H__
#define __DPSAVEDSTATE_H__

#include <Rayne/Rayne.h>

namespace DP
{
	class SavedState
	{
	public:
		SavedState();
		~SavedState();
		
		void UpdateCamera(RN::Camera *newCamera);
		
		RN::Camera *GetMainCamera() const { return _mainCamera; };
		RN::Array *GetLights() const { return _lights; }
		
	private:
		RN::Camera *_mainCamera;
		RN::Array *_cameras;
		
		RN::Array *_lights;
		RN::Array *_instancingNodes;
		
		uint32 _maxFPS;
	};
}

#endif /* __DPSAVEDSTATE_H__ */
