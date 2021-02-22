#version 330 core

out vec3 color;

in vec3 vertColour;
in vec2 vertUV;

uniform sampler2D texSampler;

void main(){
  //color = vertColour;

  color = texture(texSampler, very UV).rgb;
}