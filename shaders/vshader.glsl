#version 130

in vec4 vPosition;
in vec4 vNormal;
in int isShadow;
out vec4 color;

uniform mat4 model_view, projection, transformation;
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct, LightPosition;
uniform float shininess, attenuation_constant, attenuation_linear, attenuation_quadratic;
vec4 ambient, diffuse, specular;

void main()
{
  ambient = AmbientProduct;
  vec4 N = normalize(model_view * vNormal);
  vec4 L_temp = model_view * (LightPosition - vPosition);
  vec4 L = normalize(L_temp);
  diffuse = max(dot(L,N), 0.0) * DiffuseProduct;
  vec4 EyePoint = vec4(0.0, 0.0, 0.0, 1.0);
  vec4 V = normalize(EyePoint - (model_view * vPosition));
  vec4 H = normalize(L + V);
  specular = pow(max(dot(N, H), 0.0), shininess) * SpecularProduct;
  float distance = length(L_temp);
  float attenuation = 1/(attenuation_constant + (attenuation_linear * distance) +
    (attenuation_quadratic * distance * distance));

  color = ambient + (attenuation * (diffuse + specular));
  gl_Position = projection * model_view * transformation * vPosition / vPosition.w;
}
