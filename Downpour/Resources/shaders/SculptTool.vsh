//
//  rn_Default.vsh
//  Rayne
//
//  Copyright 2014 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#version 150
precision highp float;

#include "rn_Matrices.vsh"

in vec3 attPosition;
out vec3 vertPosition;

void main()
{
	vec4 position = vec4(attPosition, 1.0);
	vertPosition = (matModel * position).xyz;
	
	gl_Position = matProjViewModel * position;
}
