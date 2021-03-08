#version 330 core

out vec4 color;

in vec3 vertNorm;
in vec2 vertUV;

uniform sampler2D texSampler;

uniform LightBlock{
	vec3 lightDir;
	vec3 lightColour;
};
uniform vec3 objColour;

void main(){
  //color = vertColour;
  //color = vec4(0.5f, 0.5f, 0.0f, 1.0f);

  //color = texture(texSampler, vertUV);
  //color.a = 0.4f;

  //color = vec4(vertNorm, 1.0f);

  //hard coding the colours, chosing a light direction
  //vec3 lightDir = vec3(0.0f, 1.0f, 0.0f);
  //vec3 lightColour = vec3(1.0f, 1.0f, 1.0f);
  //vec3 objColour = vec3(1.0f, 1.0f, 1.0f);

  vec3 norm = normalize(vertNorm);
  vec3 lightNorm = normalize(lightDir);

  vec3 baseColour = objColour.x < 0.0f ? texture(texSampler, vertUV).xyz : objColour;

  float diffuseFactor = max(dot(norm, lightNorm), 0.0f);
  vec3 diffuse = diffuseFactor * lightColour;
  color = vec4(diffuse * baseColour, 1.0f);
}