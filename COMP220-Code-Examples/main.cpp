#include <iostream>
#include <SDL.h>
#include <gl\glew.h>
#include <SDL_opengl.h>
#include <..\SDL2_image-2.0.5\include\SDL_image.h>

#include <fstream>
#include <sstream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

struct Vertex
{
	float x, y, z, u = 0.0f, v = 0.0f;
};

bool LoadModel(const char* filePath, std::vector<Vertex>& vertices, std::vector<unsigned>& indices)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode || !scene->HasMeshes())
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Model import failed", importer.GetErrorString(), NULL);
		return false;
	}

	//Cheat! Don't do this.
	aiMesh* mesh = scene->mMeshes[0];
	if (!mesh)
		return false;

	vertices.clear();
	indices.clear();

	vertices.resize(mesh->mNumVertices);
	for (unsigned i = 0; i < mesh->mNumVertices; i++) 
	{
		vertices[i].x = mesh->mVertices[i].x;
		vertices[i].y = mesh->mVertices[i].y;
		vertices[i].z = mesh->mVertices[i].z;
	}

	for (unsigned i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace& face = mesh->mFaces[i];
		for (unsigned j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	return !(vertices.empty() || indices.empty());
}

int main(int argc, char** argsv)
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
	//SDL_Window* window = SDL_CreateWindow("SDL2 Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_Window* window = SDL_CreateWindow("SDL2 Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 960, 720, SDL_WINDOW_OPENGL);
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

	std::vector<Vertex> vertices;
	std::vector<unsigned> indices;

	if (!LoadModel("cube.nff", vertices, indices)) 
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error loading model", "", NULL);
		//TODO should clean up here
		return 1;
	}

	SDL_Surface* image = IMG_Load("Crate.jpg");
	if (!image) 
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "IMG_Load failed", IMG_GetError(), NULL);

		//Destroy the window and quit SDL2, NB we should do this after all cleanup in this order!!!
		//https://wiki.libsdl.org/SDL_DestroyWindow
		SDL_DestroyWindow(window);
		//https://wiki.libsdl.org/SDL_Quit
		SDL_Quit();

		return 1;
	}

	SDL_SetRelativeMouseMode(SDL_TRUE);

	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unable to initialise GLEW", (char*)glewGetErrorString(glewError), NULL);
	}

	//Create VAO
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	/*
	// An array of 3 vectors which represents 3 vertices
	static const GLfloat g_vertex_buffer_data[] = {
		//pos					//col
	   -1.0f, -1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
	   1.0f, -1.0f, 0.0f,		0.0f, 1.0f, 0.0f,
	   0.0f,  1.0f, 0.0f,		0.0f, 0.0f, 1.0f,
	};
	*/

	/*
	// An array of 4 vectors which represents 4 vertices
	static const GLfloat g_vertex_buffer_data[] = {
		// position			    // colour			//texture
		-0.75f, -0.75f, 0.0f,	1.0f, 0.0f, 0.0f,	0.0f, 0.0f,    // Vertex B
		0.75f, -0.75f, 0.0f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,    // Vertex C
		0.75f,  0.75f, 0.0f,	0.0f, 0.0f, 1.0f,	1.0f, 1.0f,    // Vertex D
		-0.75f, 0.75f, 0.0f,	1.0f, 1.0f, 0.0f,	0.0f, 1.0f     // Vertex A
	};

	// Triangle indices - note anticlockwise ordering!
	static const GLuint g_vertex_indices[] = {
		0, 1, 2,	// first triangle, BCD
		2, 3, 0		// second triangle, DAB
	};
	*/

	// This will identify our vertex buffer
	GLuint vertexbuffer;
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// 1st attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		sizeof(Vertex),                  // stride
		(void*)0            // array buffer offset
	);

	/*
	// 2nd attribute buffer : colour
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		8 * sizeof(GL_FLOAT),                  // stride
		(void*)(3 * sizeof(GL_FLOAT))            // array buffer offset
	);

	// 3rd attribute buffer : texture
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		8 * sizeof(GL_FLOAT),                  // stride
		(void*)(6 * sizeof(GL_FLOAT))            // array buffer offset
	);
	*/

	// Generate a buffer for the indices, element buffer
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * indices.size(), &indices[0], GL_STATIC_DRAW);

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	int Mode = GL_RGB;

	if (image->format->BytesPerPixel == 4) {
		Mode = GL_RGBA;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, Mode, image->w, image->h, 0, Mode, GL_UNSIGNED_BYTE, image->pixels);

	// Nice trilinear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Shader setup
	GLuint programID = LoadShaders("vertShader.glsl", "fragShader.glsl");

	//transforms
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
	model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));

	glm::mat4 mvp, view, projection;
	glm::vec3 position(0, 0, 5), forward, rotation(0);
	const glm::vec4 cameraFace(0, 0, -1, 0);
	const float walkSpeed = 0.5, rotSpeed = 0.1f;

	unsigned int transformLoc = glGetUniformLocation(programID, "transform");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
			case SDL_MOUSEMOTION:
			{
				rotation.y -= ev.motion.xrel * rotSpeed;
				rotation.x -= ev.motion.yrel * rotSpeed;
				glm::mat4 viewRotate(1.0f);
				viewRotate = glm::rotate(viewRotate, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
				viewRotate = glm::rotate(viewRotate, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
				forward = glm::normalize(glm::vec3(viewRotate * cameraFace));
				break;
			}
				//KEYDOWN Message, called when a key has been pressed down
			case SDL_KEYDOWN:
				//Check the actual key code of the key that has been pressed
				switch (ev.key.keysym.sym)
				{
					//Escape key
				case SDLK_ESCAPE:
					running = false;
					break;
				case SDLK_w:
					position += walkSpeed * forward;
					break;
				case SDLK_s:
					position -= walkSpeed * forward;
					break;
				}
			}
		}

		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(programID);

		view = glm::lookAt(
			position,
			position + forward,
			glm::vec3(0, 1, 0)
		);

		projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

		mvp = projection * view * model;

		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(mvp));

		// Draw the triangle !
		//glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);

		SDL_GL_SwapWindow(window);
	}

	glDisableVertexAttribArray(0);

	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);

	SDL_FreeSurface(image);
	SDL_GL_DeleteContext(glContext);

	//Destroy the window and quit SDL2, NB we should do this after all cleanup in this order!!!
	//https://wiki.libsdl.org/SDL_DestroyWindow
	SDL_DestroyWindow(window);
	//https://wiki.libsdl.org/SDL_Quit
	SDL_Quit();

	return 0;
}