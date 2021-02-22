#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexColour;
layout(location = 2) in vec2 vertexUV;

out vec3 vertColour;
out vec2 vertUV;

uniform mat4 transform;

void main(){
	gl_Position = transform * vec4(vertexPosition_modelspace, 1.0f);

  vertColour = vertexColour;
  vertUV = vertexUV;
}