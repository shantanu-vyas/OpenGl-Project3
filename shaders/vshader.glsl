#version 130

in vec4 vPosition;
in vec4 vNormal;
in vec2 vTexCoord;
out vec2 texCoord;
out vec4 color;

uniform int isShadow;
uniform mat4 model_view, projection, transformation;
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct, LightPosition;
uniform float shininess, attenuation_constant, attenuation_linear, attenuation_quadratic;
vec4 ambient, diffuse, specular;
float x,z;
void main()
{
  if(isShadow == 0)
    {
      vec4 pos =  transformation * vPosition;
      vec4 norm =  transformation * vNormal;
      vec4 light = LightPosition;
      ambient = AmbientProduct;
      vec4 N = normalize(model_view * norm);
      vec4 L_temp = model_view * (light - pos);
      vec4 L = normalize(L_temp);
      diffuse = max(dot(L,N), 0.0) * DiffuseProduct;
      vec4 EyePoint = vec4(0.0, 0.0, 0.0, 1.0);
      vec4 V = normalize(EyePoint - (model_view * pos));
      vec4 H = normalize(L + V);
      specular = pow(max(dot(N, H), 0.0), shininess) * SpecularProduct;
      float distance = length(L_temp);
      float attenuation = 1/(attenuation_constant + (attenuation_linear * distance) +
                             (attenuation_quadratic * distance * distance));

      color = ambient + (attenuation * (diffuse + specular));
      gl_Position = projection * model_view * pos / pos.w;
      texCoord = vTexCoord;
    }
  else if(isShadow == 1)
    {
      color = vec4(0,.0,.0,1);
      vec4 pos =  transformation * vPosition;
      vec4 vLight = LightPosition;
      x = vLight.x - vLight.y*((vLight.x-pos.x)/(vLight.y-pos.y));
      if (x == x) pos.x = x;
      z = vLight.z - vLight.y*((vLight.z-pos.z)/(vLight.y-pos.y));
      if (z == z) pos.z = z;
      pos.y = .1;
      
      pos = projection * model_view * pos / pos.w;;
      gl_Position = pos;
    }
}
