#include <iostream>
#include <SDL.h>
#include <GL\glew.h>
#include <SDL_opengl.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "Shaders.h"
#include "Vertex.h"
#include "Texture.h"
#include "Model.h"
#include "Mesh.h"
#include "Light.h"

int main(int argc, char ** argsv)
{
	//Initialises the SDL Library, passing in SDL_INIT_VIDEO to only initialise the video subsystems
	//https://wiki.libsdl.org/SDL_Init
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		//Display an error message box
		//https://wiki.libsdl.org/SDL_ShowSimpleMessageBox
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL_Init failed", SDL_GetError(), NULL);
		return 1;
	}

	//Create a window, note we have to free the pointer returned using the DestroyWindow Function
	//https://wiki.libsdl.org/SDL_CreateWindow
	SDL_Window* window = SDL_CreateWindow("SDL2 Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 640, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	//Checks to see if the window has been created, the pointer will have a value of some kind
	if (window == nullptr)
	{
		//Show error
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL_CreateWindow failed", SDL_GetError(), NULL);
		//Close the SDL Library
		//https://wiki.libsdl.org/SDL_Quit
		SDL_Quit();
		return 1;
	}

	//Request 3.2 Core OpenGL
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GLContext gl_Context = SDL_GL_CreateContext(window);
	if (gl_Context == nullptr)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL_CreateContext Failed", SDL_GetError(), NULL);

		SDL_DestroyWindow(window);
		SDL_Quit();

		return 1;
	}

	//init GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		//Show error
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "GLEW Initialisation Failed", (char*)glewGetErrorString(err), NULL);
		SDL_DestroyWindow(window);
		SDL_Quit();

		return 1;
	}

	glEnable(GL_DEPTH_TEST);

	//Load Mesh
	MeshCollection * teapotMesh = new MeshCollection();
	loadMeshFromFile("utah-teapot.fbx", teapotMesh);


	GLuint diffuseTextureID = loadTextureFromFile("brick_D.png");
	GLuint normalTextureID = loadTextureFromFile("brick_N.png");
	GLuint heightTextureID = loadTextureFromFile("brick_H.png");
	GLuint specularTextureID = loadTextureFromFile("specMap.png");

	// Create and compile our GLSL program from the shaders
	//GLuint programID = LoadShaders("normalMappingVert.glsl", "normalMappingFrag.glsl");
	//GLuint programID = LoadShaders("blinnPhongVert.glsl", "blinnPhongFrag.glsl");
	GLuint programID = LoadShaders("parallaxMappingVert.glsl", "parallaxMappingFrag.glsl");
	//Set up positions for position, rotation and scale
	glm::vec3 position = glm::vec3(0.0f, -8.0f, -50.0f);
	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

	//calculate the translation, rotation and scale matrices using the above vectores
	glm::mat4 translationMatrix = glm::translate(position);
	glm::mat4 rotationMatrix = glm::rotate(rotation.x, glm::vec3(1.0f, 0.0f, 0.0f))
		*glm::rotate(rotation.y, glm::vec3(0.0f, 1.0f, 0.0f))
		*glm::rotate(rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 scaleMatrix = glm::scale(scale);

	//combine the above matrices into the model matrix (order is important!!!! - TRS)
	glm::mat4 modelMatrix = translationMatrix * rotationMatrix*scaleMatrix;

	//Set up vectors for our camera position
	glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 20.0f);
	glm::vec3 cameraLook = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	//Calculate the view matrix
	glm::mat4 viewMatrix = glm::lookAt(cameraPosition, cameraLook, cameraUp);
	//Calculate our perspective matrix
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), (float)800 / (float)640, 0.1f, 100.0f);

	//Light properties
	glm::vec4 ambientLightColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	glm::vec4 diffuseLightColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 specularLightColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec3 lightDirection = glm::vec3(0.0f, 0.0f, 1.0f);


	//Material Properties
	glm::vec4 ambientMaterialColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 diffuseMaterialColour = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	glm::vec4 specularMaterialColour= glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float specularMaterialPower = 25.0f;

	//Point light
	std::vector<PointLight> PointLights;
	PointLights.push_back({ glm::vec4(1.0f,1.0f,1.0f,1.0f),glm::vec4(1.0f,1.0f,1.0f,1.0f),glm::vec3(-5.0f,-8.0f,-40.0f) });
	PointLights.push_back({ glm::vec4(1.0f,1.0f,1.0f,1.0f),glm::vec4(1.0f,1.0f,1.0f,1.0f),glm::vec3(5.0f,-8.0f,-40.0f) });
	PointLights.push_back({ glm::vec4(1.0f,1.0f,1.0f,1.0f),glm::vec4(1.0f,1.0f,1.0f,1.0f),glm::vec3(5.0f,8.0f,-40.0f) });
	

	//Get the uniforms from the shader
	GLuint modelMatrixUniformLocation = glGetUniformLocation(programID, "modelMatrix");
	GLuint viewMatrixUniformLocation = glGetUniformLocation(programID, "viewMatrix");
	GLuint projectionMatrixUniformLocation = glGetUniformLocation(programID, "projectionMatrix");
	GLint diffuseTextureLocation = glGetUniformLocation(programID, "diffuseTexture");
	GLint specularTextureLocation = glGetUniformLocation(programID, "specularTexture");
	GLint normalTextureLocation = glGetUniformLocation(programID, "normalTexture");


	GLint ambientLightColourLocation= glGetUniformLocation(programID, "ambientLightColour");
	GLint directionalLightDiffuseColourLocation = glGetUniformLocation(programID, "directionalLight.diffuseColour");
	GLint directionalLightSpecularColourLocation = glGetUniformLocation(programID, "directionalLight.specularColour");

	GLint lightDirectionLocation= glGetUniformLocation(programID, "directionalLight.direction");
	GLint cameraPositionLocation = glGetUniformLocation(programID, "cameraPosition");


	GLint ambientMaterialColourLocation= glGetUniformLocation(programID, "ambientMaterialColour");
	GLint diffuseMaterialColourLocation = glGetUniformLocation(programID, "diffuseMaterialColour");
	GLint specularMaterialColourLocation = glGetUniformLocation(programID, "specularMaterialColour");
	GLint specularMaterialPowerLocation = glGetUniformLocation(programID, "specularMaterialPower");

	const int MAX_NO_OF_POINT_LIGHTS = 8;
	GLint pointLightDiffuseColourLocations[MAX_NO_OF_POINT_LIGHTS];
	GLint pointLightSpecularColourLocations[MAX_NO_OF_POINT_LIGHTS];
	GLint pointLightPositionLocations[MAX_NO_OF_POINT_LIGHTS];

	char characterBuffer[50];
	for (int i = 0; i < MAX_NO_OF_POINT_LIGHTS; i++)
	{
		sprintf(characterBuffer,"pointLights[%i].diffuseColour", i);
		pointLightDiffuseColourLocations[i] = glGetUniformLocation(programID, characterBuffer);
		
		sprintf(characterBuffer, "pointLights[%i].specularColour", i);
		pointLightSpecularColourLocations[i] = glGetUniformLocation(programID, characterBuffer);
		
		sprintf(characterBuffer, "pointLights[%i].position", i);
		pointLightPositionLocations[i]= glGetUniformLocation(programID, characterBuffer);
	}

	GLint numberOfPointLightsLocation = glGetUniformLocation(programID, "numberOfPointLights");



	//Event loop, we will loop until running is set to false, usually if escape has been pressed or window is closed
	bool running = true;
	//SDL Event structure, this will be checked in the while loop
	SDL_Event ev;
	while (running)
	{
		//Poll for the events which have happened in this frame
		//https://wiki.libsdl.org/SDL_PollEvent
		while (SDL_PollEvent(&ev))
		{
			//Switch case for every message we are intereted in
			switch (ev.type)
			{
				//QUIT Message, usually called when the window has been closed
			case SDL_QUIT:
				running = false;
				break;
				//KEYDOWN Message, called when a key has been pressed down
			case SDL_KEYDOWN:
				//Check the actual key code of the key that has been pressed
				switch (ev.key.keysym.sym)
				{
					//Escape key
				case SDLK_ESCAPE:
					running = false;
					break;
				case SDLK_a:
					rotation.y -= 0.1f;
					break;
				case SDLK_d:
					rotation.y += 0.1f;
					break;
				case SDLK_w:
					rotation.x -= 0.1f;
					break;
				case SDLK_s:
					rotation.x += 0.1f;
					break;
				}
			}
		}

		//update
		translationMatrix = glm::translate(position);
		rotationMatrix = glm::rotate(rotation.x, glm::vec3(1.0f, 0.0f, 0.0f))
			*glm::rotate(rotation.y, glm::vec3(0.0f, 1.0f, 0.0f))
			*glm::rotate(rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		scaleMatrix = glm::scale(scale);

		//combine the above matrices into the model matrix (order is important!!!! - TRS)
		modelMatrix = translationMatrix * rotationMatrix*scaleMatrix;

		//Do rendering here!
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(programID);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseTextureID);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularTextureID);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, normalTextureID);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, heightTextureID);

		//send the uniforms across
		glUniformMatrix4fv(modelMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(viewMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		glUniform1i(diffuseTextureLocation, 0);
		glUniform1i(specularTextureLocation, 1);
		glUniform1i(normalTextureLocation, 2);
		glUniform1i(heightTextureID, 3);

		glUniform4fv(ambientMaterialColourLocation, 1, glm::value_ptr(ambientMaterialColour));
		glUniform4fv(diffuseMaterialColourLocation, 1, glm::value_ptr(diffuseMaterialColour));
		glUniform4fv(specularMaterialColourLocation, 1, glm::value_ptr(specularMaterialColour));
		
		glUniform4fv(ambientLightColourLocation, 1, glm::value_ptr(ambientLightColour));
		glUniform4fv(directionalLightDiffuseColourLocation, 1, glm::value_ptr(diffuseLightColour));
		glUniform4fv(directionalLightSpecularColourLocation, 1, glm::value_ptr(specularLightColour));
		glUniform1f(specularMaterialPowerLocation, specularMaterialPower);

		glUniform3fv(lightDirectionLocation, 1, glm::value_ptr(lightDirection));
		glUniform3fv(cameraPositionLocation, 1, glm::value_ptr(cameraPosition));

		for (int i=0; i < PointLights.size(); i++)
		{
			glUniform4fv(pointLightDiffuseColourLocations[i], 1, glm::value_ptr(PointLights[i].DiffuseColour));
			glUniform4fv(pointLightSpecularColourLocations[i], 1, glm::value_ptr(PointLights[i].SpecularColour));
			glUniform3fv(pointLightPositionLocations[i], 1, glm::value_ptr(PointLights[i].Position));
		}

		glUniform1i(numberOfPointLightsLocation, PointLights.size());


		teapotMesh->render();

		SDL_GL_SwapWindow(window);
	}
	if (teapotMesh)
	{
		delete teapotMesh;
		teapotMesh = nullptr;
	}
	glDeleteTextures(1, &diffuseTextureID);
	glDeleteTextures(1, &normalTextureID);
	glDeleteTextures(1, &specularTextureID);

	glDeleteProgram(programID);
	//Delete Context
	SDL_GL_DeleteContext(gl_Context);
	//Destroy the window and quit SDL2, NB we should do this after all cleanup in this order!!!
	//https://wiki.libsdl.org/SDL_DestroyWindow
	SDL_DestroyWindow(window);
	//https://wiki.libsdl.org/SDL_Quit
	SDL_Quit();

	return 0;
}