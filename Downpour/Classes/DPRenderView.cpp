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
	RNDefineMeta(RenderView, RN::UI::View)
	
	RenderView::RenderView() :
		_materials(new RN::Array()),
		_shader(new RN::Shader())
	{
		SetInteractionEnabled(false);
		
		_shader->SetShaderForType("shader/rn_UIImage.vsh", RN::ShaderType::VertexShader);
		_shader->SetShaderForType("shader/rn_DrawFramebufferTonemap.fsh", RN::ShaderType::FragmentShader);
		
		
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
		
		RN::Vector2 size = GetBounds().GetSize();
		*vertices ++ = RN::Vector2(size.x, size.y);
		*vertices ++ = RN::Vector2(0.0f, size.y);
		*vertices ++ = RN::Vector2(size.x, 0.0f);
		*vertices ++ = RN::Vector2(0.0f, 0.0f);
		
		*uvCoords ++ = RN::Vector2(1.0f, 1.0f);
		*uvCoords ++ = RN::Vector2(0.0f, 1.0f);
		*uvCoords ++ = RN::Vector2(1.0f, 0.0f);
		*uvCoords ++ = RN::Vector2(0.0f, 0.0f);
		
		chunk.CommitChanges();
	}
		
	RenderView::~RenderView()
	{
		_materials->Release();
		_shader->Release();
		_mesh->Release();
	}
	
	void RenderView::SetFrame(const RN::Rect& frame)
	{
		RN::UI::View::SetFrame(frame);
		
		RN::Mesh::Chunk chunk = _mesh->GetChunk();
		RN::Mesh::ElementIterator<RN::Vector2> vertices = chunk.GetIterator<RN::Vector2>(RN::MeshFeature::Vertices);
		
		*vertices ++ = RN::Vector2(frame.width, frame.height);
		*vertices ++ = RN::Vector2(0.0f, frame.height);
		*vertices ++ = RN::Vector2(frame.width, 0.0f);
		*vertices ++ = RN::Vector2(0.0f, 0.0f);

		chunk.CommitChanges();
	}
	
	void RenderView::AddTexture(RN::Texture *texture)
	{
		RN::Material *material = BasicMaterial(_shader);
		material->Define("targetmap0", "mTexture0");
		material->AddTexture(texture);
		
		_materials->AddObject(material);
	}
	
	void RenderView::Draw(RN::Renderer *renderer)
	{
		RN::UI::View::Draw(renderer);
		
		_materials->Enumerate<RN::Material>([&](RN::Material *material, size_t index, bool &stop) {
			
			RN::RenderingObject object;
			PopulateRenderingObject(object);
			
			object.mesh     = _mesh;
			object.material = material;
			
			renderer->RenderObject(object);
			
		});
	}
}
