//
//  rn_Default.fsh
//  Rayne
//
//  Copyright 2014 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#version 150
precision highp float;

//in out and uniforms
uniform vec4 diffuse;
uniform sampler2D mTexture0;
in vec3 vertPosition;
out vec4 fragColor0;

void main()
{	
	fragColor0 = diffuse;
}
