#version 330 core

out vec4 color;

in vec4 vertColour;
in vec2 vertUV;

uniform sampler2D texSampler;

void main(){
  //color = vertColour;
  //color = vec4(0.5f, 0.5f, 0.0f, 1.0f);

  color = texture(texSampler, vertUV);
  //color.a = 0.4f;
}