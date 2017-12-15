#version 130
in vec4 color;
in vec2 texCoord;
out vec4 fColor;
uniform sampler2D texture;
uniform int isTexture;
void main()
{
  if (isTexture == 1)
    {
      fColor = color;
    }
  else if (isTexture == 0)
    {
      fColor = texture2D(texture, texCoord);
    }
}
