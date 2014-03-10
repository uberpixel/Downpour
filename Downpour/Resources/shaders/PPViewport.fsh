//
//  PPViewport.fsh
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

#version 150
precision highp float;

uniform sampler2D targetmap0; // Editor camera

uniform sampler2D mTexture0; // Normal depth
uniform sampler2D mTexture1; // Editor depth

in vec2 vertTexcoord;
out vec4 fragColor0;

vec3 desaturate(vec3 color, float amount)
{
    vec3 gray = vec3(dot(vec3(0.2126, 0.7152 ,0.0722), color));
    return vec3(mix(color, gray, amount));
}

void main()
{
	vec4 color = texture(targetmap0, vertTexcoord);

	float depth0 = texture(mTexture0, vertTexcoord).r;
	float depth1 = texture(mTexture1, vertTexcoord).r;

	if(depth0 < depth1)
	{
		color.rgb = desaturate(color.rgb, 0.4);
		color.a *= 0.6;
	}

	fragColor0 = color;
}