#version 130

in vec4 vPosition;
in vec4 vColor;

uniform vec4 vLight;
uniform int isShadow;
uniform mat4 projection, model_view, transformation;

out vec4 color;

void main()
{
  if (isShadow == 0)
    {
      color = vColor;
      gl_Position = projection * model_view * transformation * vPosition / vPosition.w;
    }

  else
    {
      color = vec4(0,.0,.0,1);
      vec4 pos =  transformation * vPosition;
      
      pos.x = vLight.x - vLight.y*((vLight.x-pos.x)/(vLight.y-pos.y));
      pos.z = vLight.z - vLight.y*((vLight.z-pos.z)/(vLight.y-pos.y));
      pos.y = .1;
      
      pos = projection * model_view * pos / pos.w;;
      gl_Position = pos;
    }
}
