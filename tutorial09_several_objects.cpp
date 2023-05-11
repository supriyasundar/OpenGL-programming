/*
Author: Supriya Sundar
Class: ECE 6122
Last Date Modified: 12/7/2021
Description: Simulation of motion of uav objects towards spherical surface was implemented using laws of physics and kinematics.
References: 1.Professor's explained code snippets from lectures
2.https://learnopengl.com/In-Practice/2D-Game/Collisions/Collision-detection
3.https://www.cprogramming.com/tutorial/opengl_first_opengl_program.html*/

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include<mutex>
#include<atomic>
#include<chrono>
#include<thread>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

#include "..\bin\ECE_UAV.h" // C drive

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

int main(void)
{
	ECE_UAV uavs[15]; // initialise array for 15 uavs
	double initialX = -3200.0f; //initialising every individual uav object
	double initialZ = 500.0f;
	double offsetX = 1100.0f;
	double offsetZ = 250.0f;
	double xPosition = initialX;
	double zPosition = initialZ;
	double uavPosition[3] = { 0.0,100.0f,0.0f };
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			xPosition += offsetX;
			uavPosition[0] = xPosition;
			uavPosition[2] = zPosition;
			uavs[i * 5 + j].position(uavPosition);
		}
		zPosition += offsetZ;
		xPosition = -3200.0f;
	}

	clock_t start = clock(); //time to keep track of simulation duration

	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Tutorial 09 - Rendering several models", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Load the texture
	GLuint Texture_uav = loadDDS("uvmap.DDS");
	GLuint Texture = loadBMP_custom("ff.bmp");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	//setting up football field
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("footballfield.obj", vertices, uvs, normals);

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	// Load it into a VBO

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	//setting up sphere
	// Read our .obj file
	std::vector<glm::vec3> vertices_sphere;
	std::vector<glm::vec2> uvs_sphere;
	std::vector<glm::vec3> normals_sphere;
	bool res1 = loadOBJ("sphere.obj", vertices_sphere, uvs_sphere, normals_sphere);

	std::vector<unsigned short> indices_sphere;
	std::vector<glm::vec3> indexed_vertices_sphere;
	std::vector<glm::vec2> indexed_uvs_sphere;
	std::vector<glm::vec3> indexed_normals_sphere;
	indexVBO(vertices_sphere, uvs_sphere, normals_sphere, indices_sphere, indexed_vertices_sphere, indexed_uvs_sphere, indexed_normals_sphere);

	// Load it into a VBO
	GLuint vertexbuffer_sphere;
	glGenBuffers(1, &vertexbuffer_sphere);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_sphere);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices_sphere.size() * sizeof(glm::vec3), &indexed_vertices_sphere[0], GL_STATIC_DRAW);

	GLuint uvbuffer_sphere;
	glGenBuffers(1, &uvbuffer_sphere);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_sphere);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs_sphere.size() * sizeof(glm::vec2), &indexed_uvs_sphere[0], GL_STATIC_DRAW);

	GLuint normalbuffer_sphere;
	glGenBuffers(1, &normalbuffer_sphere);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer_sphere);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals_sphere.size() * sizeof(glm::vec3), &indexed_normals_sphere[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	GLuint elementbuffer_sphere;
	glGenBuffers(1, &elementbuffer_sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer_sphere);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_sphere.size() * sizeof(unsigned short), &indices_sphere[0], GL_STATIC_DRAW);

	//setting up uav
	// Read our .obj file
	std::vector<glm::vec3> vertices_uav;
	std::vector<glm::vec2> uvs_uav;
	std::vector<glm::vec3> normals_uav;
	bool res2 = loadOBJ("duck-float.obj", vertices_uav, uvs_uav, normals_uav);

	std::vector<unsigned short> indices_uav;
	std::vector<glm::vec3> indexed_vertices_uav;
	std::vector<glm::vec2> indexed_uvs_uav;
	std::vector<glm::vec3> indexed_normals_uav;
	indexVBO(vertices_uav, uvs_uav, normals_uav, indices_uav, indexed_vertices_uav, indexed_uvs_uav, indexed_normals_uav);

	// Load it into a VBO

	GLuint vertexbuffer_uav;
	glGenBuffers(1, &vertexbuffer_uav);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_uav);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices_uav.size() * sizeof(glm::vec3), &indexed_vertices_uav[0], GL_STATIC_DRAW);

	GLuint uvbuffer_uav;
	glGenBuffers(1, &uvbuffer_uav);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_uav);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs_uav.size() * sizeof(glm::vec2), &indexed_uvs_uav[0], GL_STATIC_DRAW);

	GLuint normalbuffer_uav;
	glGenBuffers(1, &normalbuffer_uav);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer_uav);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals_uav.size() * sizeof(glm::vec3), &indexed_normals_uav[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	GLuint elementbuffer_uav;
	glGenBuffers(1, &elementbuffer_uav);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer_uav);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_uav.size() * sizeof(unsigned short), &indices_uav[0], GL_STATIC_DRAW);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	//uav position initialize and start
	float angle = (90.0f / (2.0f * 3.14156f));
	for (int i = 0; i < 15; i++)
	{
		uavs[i].start();
	}

	do {

		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();

		glm::mat4 ModelMatrix1 = glm::mat4(1.0);
		ModelMatrix1 = glm::scale(ModelMatrix1, glm::vec3(0.85f, 0.85f, 0.85f));
		ModelMatrix1 = glm::rotate(ModelMatrix1, angle, glm::vec3(-1.0f, 0.0f, 0.0f));
		glm::mat4 MVP1 = ProjectionMatrix * ViewMatrix * ModelMatrix1;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP1[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix1[0][0]);

		////// Start of the rendering of the first object //////
		glm::vec3 lightPos = glm::vec3(0, 110, 110);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);// This one doesn't change between objects, so this can be done once for all objects that use "programID"

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,      // mode
			indices.size(),    // count
			GL_UNSIGNED_SHORT,   // type
			(void*)0           // element array buffer offset
		);

		////// End of rendering of the first object //////
		////// Start of the rendering of the second object //////
		glm::mat4 ModelMatrix2 = glm::mat4(1.0);
		ModelMatrix2 = glm::scale(ModelMatrix2, glm::vec3(8.0f, 8.0f, 8.0f));
		ModelMatrix2 = glm::translate(ModelMatrix2, glm::vec3(0.0f, 10.0f, 0.0f));
		glm::mat4 MVP2 = ProjectionMatrix * ViewMatrix * ModelMatrix2;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix2[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		// The rest is exactly the same as the first object
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture_uav);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_sphere);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_sphere);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer_sphere);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer_sphere);

		// Draw the triangles !
		glDrawElements(GL_TRIANGLES, indices_sphere.size(), GL_UNSIGNED_SHORT, (void*)0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_uav);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_uav);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer_uav);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer_uav);

		double pos[3] = { 0.0,0.0,0.0 };
		for (int i = 0; i < 15; i++)
		{
			uavs[i].getPosition(pos);
			glm::mat4 ModelMatrix3 = glm::mat4(1.0);
			ModelMatrix3 = glm::scale(ModelMatrix3, glm::vec3(0.05f, 0.05f,0.05f));
			ModelMatrix3 = glm::translate(ModelMatrix3, glm::vec3(pos[0], pos[1], pos[2]));
			glm::mat4 MVP3 = ProjectionMatrix * ViewMatrix * ModelMatrix3;

			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP3[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix3[0][0]);
			glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

			glDrawElements(GL_TRIANGLES, indices_uav.size(), GL_UNSIGNED_SHORT, (void*)0);

		}

		////// End of rendering of the second object //////
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	for (int i = 0; i < 15; i++)
	{
		uavs[i].stop();
	}

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteBuffers(1, &vertexbuffer_sphere);
	glDeleteBuffers(1, &uvbuffer_sphere);
	glDeleteBuffers(1, &normalbuffer_sphere);
	glDeleteBuffers(1, &elementbuffer_sphere);
	glDeleteBuffers(1, &vertexbuffer_uav);
	glDeleteBuffers(1, &uvbuffer_uav);
	glDeleteBuffers(1, &normalbuffer_uav);
	glDeleteBuffers(1, &elementbuffer_uav);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;

	/*if ((clock() - start) / CLOCKS_PER_SEC >= 85) //exit simulation if time is more than 1 min
	{
		break; 
	}*/
}

