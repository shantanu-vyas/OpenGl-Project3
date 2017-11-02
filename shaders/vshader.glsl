#version 130

in vec4 vPosition;
in vec4 vColor;
out vec4 color;

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform mat4 transformation_matrix;

void main()
{
	color = vColor;
	gl_Position = transformation_matrix*projection_matrix*modelview_matrix*vPosition/vPosition.w;
}