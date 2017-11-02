#version 130

in vec4 color;
out vec4 fColor;

void main()
{
  fColor = color;
  //gl_FragColor = vec4(1.0,0,0,0);
}
