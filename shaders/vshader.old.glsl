#version 130

in vec4 vPosition;
in vec4 vColor;
in vec4 light_vertex
in int drawShadows;
out vec4 color;

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform mat4 transformation_matrix;

void main()
{
  if (drawShadows == 1){ 
    color = vColor;
    gl_Position = projection_matrix*modelview_matrix*transformation_matrix*vPosition/vPosition.w;
  }
  else { // draw shadow
    vec4 temp = projection_matrix*modelview_matrix*transformation_matrix*vPosition/vPosition.w;
    color = {0.1f, 0.lf, 0.1f, 1.f};
    gl_Position = genShadows(asdfasdf  
  }
}
