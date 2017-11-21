#version 130

in vec4 vPosition;
in vec4 vLight;
in vec4 vColor;
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
      color = vec4(1,0,1,1);
      vec4 pos = vPosition;
      
       pos.y = vLight.x - vLight.y*((vLight.x-pos.x)/(vLight.y-pos.y))+.1;  
       //pos.x = vLight.x - vLight.y*((vLight.x - pos.x) / (vLight.y - pos.y)); 
      /* pos.y = 0; */
      /* pos.z = vLight.z - vLight.y*((vLight.z - pos.z) / (vLight.y - posw.y)); */
      pos.y += 2;
      
      //pos.y = vLight.x - vLight.y*((vLight.x-pos.x)/(vLight.y-pos.y))+.1;
      
      pos = projection * model_view * transformation * pos / pos.w;;
      gl_Position = pos;
    }
}
