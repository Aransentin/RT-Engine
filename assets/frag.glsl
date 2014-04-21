#version 140
precision highp float;

uniform sampler2D mtex;

in vec2 oUV;
out vec4 fCol;

void main()
{
	fCol = vec4( texture( mtex, oUV ).rgb, 1.0 );
}