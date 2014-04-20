#version 140
precision highp float;

in vec2 vPos;
in vec2 vUV;

out vec2 oUV;

void main()
{
	oUV = vUV;
	gl_Position = vec4( vPos, 0.0, 1.0 );
}
