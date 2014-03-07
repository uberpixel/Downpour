//
//  DPRenderView.cpp
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

#include "DPRenderView.h"

namespace DP
{
	RNDefineMeta(RenderView)
	
	RenderView::RenderView()
	{
		Initialize();
	}
		
	RenderView::~RenderView()
	{
		if(_mesh)
			_mesh->Release();
		
		if(_texture)
			_texture->Release();
		
		_material->Release();
	}
	
	
	void RenderView::Initialize()
	{
		SetInteractionEnabled(false);
		SetBackgroundColor(RN::Color::ClearColor());
		
		_mesh  = nullptr;
		RN::Shader *shader = new RN::Shader();
		shader->SetShaderForType("shader/rn_UIImage.vsh", RN::ShaderType::VertexShader);
		shader->SetShaderForType("shader/rn_DrawFramebufferTonemap.fsh", RN::ShaderType::FragmentShader);
		_material = BasicMaterial(shader);
		_material->Retain();
		_material->Define("targetmap0", "mTexture0");
		
		_texture   = nullptr;
		_isDirty = true;
		_scaleMode = RN::UI::ScaleMode::AxisIndependently;
	}
	
	RN::Vector2 RenderView::GetSizeThatFits()
	{
		if(!_texture)
			return RN::Vector2(0.0f, 0.0f);
		
		return RN::Vector2(_texture->GetWidth(), _texture->GetHeight());
	}
	
	void RenderView::SetFrame(const RN::Rect& frame)
	{
		View::SetFrame(frame);
		_isDirty = true;
	}
	
	void RenderView::SetScaleMode(RN::UI::ScaleMode mode)
	{
		_scaleMode = mode;
		_isDirty = true;
	}
	
	void RenderView::SetTexture(RN::Texture *texture)
	{			
		if(_texture)
		{
			_texture->Release();
			_texture = nullptr;
		}
		
		_texture = texture ? texture->Retain() : nullptr;
		_isDirty = true;
	}
	
	void RenderView::Update()
	{
		RN::UI::View::Update();
		
		if(_isDirty)
		{
			if(_mesh)
			{
				_mesh->Release();
				_mesh = nullptr;
			}
			
			if(_texture)
			{
				_material->RemoveTextures();
				_material->AddTexture(_texture);
			}
				
			RN::MeshDescriptor vertexDescriptor(RN::MeshFeature::Vertices);
			vertexDescriptor.elementMember = 2;
			vertexDescriptor.elementSize   = sizeof(RN::Vector2);
			
			RN::MeshDescriptor uvDescriptor(RN::MeshFeature::UVSet0);
			uvDescriptor.elementMember = 2;
			uvDescriptor.elementSize   = sizeof(RN::Vector2);
			
			std::vector<RN::MeshDescriptor> descriptors = { vertexDescriptor, uvDescriptor };
			_mesh = new RN::Mesh(descriptors, 4, 0);
			_mesh->SetDrawMode(RN::Mesh::DrawMode::TriangleStrip);
			
			RN::Mesh::Chunk chunk = _mesh->GetChunk();
			
			RN::Mesh::ElementIterator<RN::Vector2> vertices = chunk.GetIterator<RN::Vector2>(RN::MeshFeature::Vertices);
			RN::Mesh::ElementIterator<RN::Vector2> uvCoords = chunk.GetIterator<RN::Vector2>(RN::MeshFeature::UVSet0);
			
			RN::Vector2 size = GetBounds().Size();
			*vertices ++ = RN::Vector2(size.x, size.y);
			*vertices ++ = RN::Vector2(0.0f, size.y);
			*vertices ++ = RN::Vector2(size.x, 0.0f);
			*vertices ++ = RN::Vector2(0.0f, 0.0f);
			
			*uvCoords ++ = RN::Vector2(1.0f, 1.0f);
			*uvCoords ++ = RN::Vector2(0.0f, 1.0f);
			*uvCoords ++ = RN::Vector2(1.0f, 0.0f);
			*uvCoords ++ = RN::Vector2(0.0f, 0.0f);
			
			chunk.CommitChanges();
			
			_mesh->Retain()->Autorelease();
			
			_isDirty = false;
		}
	}
	
	void RenderView::Draw(RN::Renderer *renderer)
	{
		View::Draw(renderer);
		
		if(_mesh)
		{
			RN::RenderingObject object;
			PopulateRenderingObject(object);
			
			object.mesh = _mesh;
			object.material = _material;
			
			renderer->RenderObject(object);
		}
	}
}
