#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexColour;

out vec3 vertColour;

uniform mat4 transform;

void main(){
	gl_Position = transform * vec4(vertexPosition_modelspace, 1.0f);

  vertColour = vertexColour;
}