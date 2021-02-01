#version 330 core

out vec3 color;

in vec3 vertColour;

void main(){
  color = vertColour;
}