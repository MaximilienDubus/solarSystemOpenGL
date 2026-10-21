// main.cpp

#define _USE_MATH_DEFINES

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> // vec3, mat4, ...
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>  // for transformation matrices

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>

#include "mesh.h"
#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef _MY_OPENGL_IS_33_
#define _MY_OPENGL_IS_33_
#endif

struct Material {
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

    float alpha = 8.0f;

    float ka = 0.2f;
    float kd = 0.8f;
    float ks = 0.5f;
};


// constants
const static float kSizeSun = 1;
const static float kSizeEarth = 0.5;
const static float kSizeMoon = 0.25;

const static float kDistanceSunEarth = 3;
const static float kDistanceEarthMoon = 1;

const static float kRadRotationSun = 2.0f;  // rotation period of Sun
const static float kRadOrbitEarth = kRadRotationSun * 2.0f;  // orbital period of earth (around Sun)
const static float kRadRotationEarth = kRadOrbitEarth / 2.0f;  // rotation period of earth
const static float kRadOrbitMoon = kRadRotationEarth / 2.0f;  // orbital period of moon (around Earth)
const static float kRadRotationMoon = kRadOrbitMoon;  // rotation period of moon

const float kRadEarthInclinationAngle(glm::radians(23.5f));

// Model transformation matrices
glm::mat4 g_sun;
glm::mat4 g_earth;
glm::mat4 g_moon;

// materials
Material sunMaterial = {glm::vec3(1.0, 1.0, 0.0), 16.0, 1.0, 0.0, 0.0};
Material earthMaterial = {glm::vec3(0.2, 0.2, 1.0)};
Material moonMaterial = {glm::vec3(1.0, 1.0, 1.0)};

// texture
GLuint g_earthTexID;
GLuint g_moonTexID;

// Window parameters
GLFWwindow *g_window = nullptr;

// GPU objects
GLuint g_program = 0; // A GPU program contains at least a vertex shader and a fragment shader

// Sphere Mesh
std::shared_ptr<Mesh> sphere = Mesh::genSphere(64);

// Camera
Camera g_camera;
float zoom = 1.0f; // fov*zoom


int matColorUniformLocation = 0; // uniform location of the material color in the GPU program
int matKaUniformLocation = 0; // uniform location of the material ka in the GPU program
int matKdUniformLocation = 0; // uniform location of the material kd in the GPU program
int matKsUniformLocation = 0; // uniform location of the material ks in the GPU program
int matAlphaUniformLocation = 0; // uniform location of the material alpha in the GPU program
int matAlbedoTexUniformLocation = 0; // uniform location of the material albedoTex in the GPU program
int useAlbedoTexUniformLocation = 0; // uniform location of the useAlbedoTex in the GPU program
int diffuseLightColorUniformLocation = 0; // uniform location of the diffuse light color in the GPU program
int specularLightColorUniformLocation = 0; // uniform location of the specular light color in the GPU program
int ambientLightColorUniformLocation = 0; // uniform location of the ambient light color in the GPU program
int lightPosUniformLocation = 0; // uniform location of the light position in the GPU program
int worldMatUniformLocation = 0; // uniform location of the world matrix in the GPU program
int viewMatUniformLocation = 0; // uniform location of the view matrix in the GPU program
int projMatUniformLocation = 0; // uniform location of the projection matrix in the GPU program
int camPosUniformLocation = 0; // uniform location of the camera position in the GPU program
int fNormalUniformLocation = 0; // uniform location of the normal in the GPU program
int fTexCoordUniformLocation = 0; // uniform location of the texture coordinate in the GPU program
int fWorldPositionUniformLocation = 0; // uniform location of the world position in the GPU program
int useAlbedoTextureUniformLocation = 0; // uniform location of the useAlbedoTexture in the GPU program
int materialUniformLocation = 0; // uniform location of the material in the GPU program


GLuint loadTextureFromFileToGPU(const std::string &filename) {
  // Loading the image in CPU memory using stb_image
  int width, height, numComponents;
  // stbi_set_flip_vertically_on_load(true);
  
  unsigned char *data = stbi_load(filename.c_str(), &width, &height, &numComponents, 0);
  if (!data) {
    std::cerr << "Failed to load texture: " << filename << "\n" << stbi_failure_reason() << std::endl;
    return 0;
  }

  GLuint texID; // OpenGL texture identifier
  glGenTextures(1, &texID); // generate an OpenGL texture container
  glBindTexture(GL_TEXTURE_2D, texID); // activate the texture

  // Setup the texture filtering option and repeat mode; check www.opengl.org for details.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Fill the GPU texture with the data stored in the CPU image
  GLenum fmt = (numComponents == 4) ? GL_RGBA : (numComponents == 1) ? GL_RED : GL_RGB;
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // important pour les largeurs non multiples de 4 en RGB
  glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, fmt, GL_UNSIGNED_BYTE, data);
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

  // Free useless CPU memory
  stbi_image_free(data);
  glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture
  return texID;
}



// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback(GLFWwindow* window, int width, int height) {
  g_camera.setAspectRatio(static_cast<float>(width)/static_cast<float>(height));
  glViewport(0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}



// Executed each time a key is entered.
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if(action == GLFW_PRESS && key == GLFW_KEY_W) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else if(action == GLFW_PRESS && key == GLFW_KEY_F) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  } else if(action == GLFW_PRESS && (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)) {
    glfwSetWindowShouldClose(window, true); // Closes the application if the escape key is pressed
  }
}



void errorCallback(int error, const char *desc) {
  std::cout <<  "Error " << error << ": " << desc << std::endl;
}



void initGLFW() {
  glfwSetErrorCallback(errorCallback);

  // Initialize GLFW, the library responsible for window management
  if(!glfwInit()) {
    std::cerr << "ERROR: Failed to init GLFW" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // Before creating the window, set some option flags
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

  // Create the window
  g_window = glfwCreateWindow(
    1024, 768,
    "Introduction to Computer Graphics - Simple Solar System",
    nullptr, nullptr);
  if(!g_window) {
    std::cerr << "ERROR: Failed to open window" << std::endl;
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  // Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
  glfwMakeContextCurrent(g_window);
  glfwSetWindowSizeCallback(g_window, windowSizeCallback);
  glfwSetKeyCallback(g_window, keyCallback);
}



void initOpenGL() {
  // Load extensions for modern OpenGL
  if(!gladLoadGL(glfwGetProcAddress)) {
    std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  glCullFace(GL_BACK); // Specifies the faces to cull (here the ones pointing away from the camera)
  glEnable(GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
  glDepthFunc(GL_LESS);   // Specify the depth test for the z-buffer
  glEnable(GL_DEPTH_TEST);      // Enable the z-buffer test in the rasterization
  glClearColor(0.7f, 0.7f, 0.7f, 1.0f); // specify the background color, used any time the framebuffer is cleared
}



// Loads the content of an ASCII file in a standard C++ string
std::string file2String(const std::string &filename) {
  std::ifstream t(filename.c_str());
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}



// Loads and compile a shader, before attaching it to a program
void loadShader(GLuint program, GLenum type, const std::string &shaderFilename) {
  GLuint shader = glCreateShader(type); // Create the shader, e.g., a vertex shader to be applied to every single vertex of a mesh
  std::string shaderSourceString = file2String(shaderFilename); // Loads the shader source from a file to a C++ string
  const GLchar *shaderSource = (const GLchar *)shaderSourceString.c_str(); // Interface the C++ string through a C pointer
  glShaderSource(shader, 1, &shaderSource, NULL); // load the vertex shader code
  glCompileShader(shader);
  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if(!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cout << "ERROR in compiling " << shaderFilename << "\n\t" << infoLog << std::endl;
  }
  glAttachShader(program, shader);
  glDeleteShader(shader);
}


// Loads the vertex and fragment shaders, link them into a GPU program, and send the texture to the GPU
void initGPUprogram() {
  g_program = glCreateProgram(); // Create a GPU program, i.e., two central shaders of the graphics pipeline
  loadShader(g_program, GL_VERTEX_SHADER, "vertexShader.glsl");
  loadShader(g_program, GL_FRAGMENT_SHADER, "fragmentShader.glsl");

  glLinkProgram(g_program); // The main GPU program is ready to be handle streams of polygons

  matKaUniformLocation = glGetUniformLocation(g_program, "material.ka");
  matKsUniformLocation = glGetUniformLocation(g_program, "material.ks");
  matKdUniformLocation = glGetUniformLocation(g_program, "material.kd");
  matAlphaUniformLocation = glGetUniformLocation(g_program, "material.alpha");
  matColorUniformLocation = glGetUniformLocation(g_program, "material.color");


  glUseProgram(g_program);

  g_earthTexID = loadTextureFromFileToGPU("media/earth.jpg");
  g_moonTexID = loadTextureFromFileToGPU("media/moon.jpg");
  glUniform1i(glGetUniformLocation(g_program, "material.albedoTex"), 0); // texture unit 0
}

// Initialize the camera parameters based on the current window size
void initCamera() {
  int width, height;
  glfwGetWindowSize(g_window, &width, &height);
  g_camera.setAspectRatio(static_cast<float>(width)/static_cast<float>(height));

  g_camera.setPosition(glm::vec3(0.0f, 2.0f, 7.0f));
  g_camera.setNear(0.1f);
  g_camera.setFar(80.1f);
  g_camera.setFoV(g_camera.getFov() * zoom);
}


// Initialize all the GPU and CPU resources
void init() {
  initGLFW();
  initOpenGL();
  // initCPUgeometry();
  sphere->init();
  initGPUprogram();
  // initGPUgeometry();
  initCamera();
}


// Free all the GPU and CPU resources
void clear() {

  glDeleteProgram(g_program);

  sphere->clear();
  glDeleteTextures(1, &g_earthTexID);
  glDeleteTextures(1, &g_moonTexID);

  glfwDestroyWindow(g_window);
  glfwTerminate();
}

void drawSphere(const glm::mat4 worldMatrix, const Material& material, bool useTexture) {

  glUniformMatrix4fv(glGetUniformLocation(g_program, "worldMat"), 1, GL_FALSE, glm::value_ptr(worldMatrix));
  
  glUniform3fv(matColorUniformLocation, 1, glm::value_ptr(material.color));
  glUniform1f(glGetUniformLocation(g_program, "material.ka"), material.ka);
  glUniform1f(glGetUniformLocation(g_program, "material.kd"), material.kd);
  glUniform1f(glGetUniformLocation(g_program, "material.ks"), material.ks);
  glUniform1f(glGetUniformLocation(g_program, "material.alpha"), material.alpha);

  glUniform1i(glGetUniformLocation(g_program, "useAlbedoTexture"), useTexture ? GL_TRUE : GL_FALSE);

  sphere->render();
}


// The main rendering call
void render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.

  // getting camera infos
  const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
  const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();
  const glm::vec3 camPosition = g_camera.getPosition();

  // Sending camera info in the vertexShader 
  glUniformMatrix4fv(glGetUniformLocation(g_program, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix)); // compute the view matrix of the camera and pass it to the GPU program
  glUniformMatrix4fv(glGetUniformLocation(g_program, "projMat"), 1, GL_FALSE, glm::value_ptr(projMatrix)); // compute the projection matrix of the camera and pass it to the GPU program
  glUniform3f(glGetUniformLocation(g_program, "camPos"), camPosition[0], camPosition[1], camPosition[2]);

  // sending global light info
  glm::vec3 lightColor = glm::vec3(1.0, 1.0, 1.0);  // for now a white light for the three component of the light
  glUniform3fv(glGetUniformLocation(g_program, "diffuseLightColor"), 1, glm::value_ptr(lightColor));
  glUniform3fv(glGetUniformLocation(g_program, "specularLightColor"), 1, glm::value_ptr(lightColor));
  glUniform3fv(glGetUniformLocation(g_program, "ambientLightColor"), 1, glm::value_ptr(lightColor));
  glUniform3fv(glGetUniformLocation(g_program, "lightPos"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));

  // sending texture
  glActiveTexture(GL_TEXTURE0); // activate texture unit 0

  // Astres
  glBindTexture(GL_TEXTURE_2D, 0); // no texture
  drawSphere(g_sun, sunMaterial, false);
  glBindTexture(GL_TEXTURE_2D, g_earthTexID);
  drawSphere(g_earth, earthMaterial, true);
  glBindTexture(GL_TEXTURE_2D, g_moonTexID);
  drawSphere(g_moon, moonMaterial, true);
}


/*
IN RADIAN : float angle = glm::radians(90.0f);  
Function, Signature
glm::translate, "(mat4, vec3)"
glm::rotate, "(mat4, float, vec3 axe)"
glm::scale, "(mat4, vec3)"

What is left or inside is applied in first

*/

// Update any accessible variable based on the current time
void update(const float t) {
  // currentTimeInSec : time from beginning of execution, precise at 1/10 ms

  const glm::mat4 I(1.0f); // identity mat
  const glm::vec3 up(0.0f, 1.0f, 0.0f); // up vector
  const glm::vec3 z(0.0f, 0.0f, 1.0f);

  // Sun : rotate -> scale
  g_sun = glm::rotate(I, t/kRadRotationSun, up) * glm::scale(I, glm::vec3(kSizeSun));

  // Rotation of earth around Sun
  const glm::mat4 earthRevolution = glm::rotate(I, t/kRadOrbitEarth, up);
  // Translation of earth from Sun
  const glm::mat4 earthTranslation = glm::translate(I, glm::vec3(kDistanceSunEarth, 0.0f, 0.0f));
  // cancel the effect of Earth revolution on Earth rotation
  const glm::mat4 cancelEarthRevolution = glm::rotate(I, -t/kRadOrbitEarth, up);
  // Inclinaison de l'axe terrestre
  const glm::mat4 earthInclination = glm::rotate(I, kRadEarthInclinationAngle, z);
  // Rotation of earth around itself
  const glm::mat4 earthRotation = glm::rotate(I, t/kRadRotationEarth, up);

  g_earth = earthRevolution * earthTranslation * cancelEarthRevolution * earthInclination * earthRotation * glm::scale(I, glm::vec3(kSizeEarth));

  const glm::mat4 moonRevolution = glm::rotate(I, t/kRadOrbitMoon, up);
  const glm::mat4 moonTranslation = glm::translate(I, glm::vec3(kDistanceEarthMoon, 0.0f, 0.0f));
  const glm::mat4 cancelMoonRevolution = glm::rotate(I, -t/kRadOrbitMoon, up);
  const glm::mat4 moonRotation = glm::rotate(I, t/kRadRotationMoon, up);

  g_moon = earthRevolution * earthTranslation * cancelEarthRevolution * moonRevolution * moonTranslation * cancelMoonRevolution * moonRotation * glm::scale(I, glm::vec3(kSizeMoon));
}



int main(int argc, char ** argv) {
  init(); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)

  while(!glfwWindowShouldClose(g_window)) {
    update(static_cast<float>(glfwGetTime()));
    render();
    glfwSwapBuffers(g_window);
    glfwPollEvents();
  }
  clear();
  return EXIT_SUCCESS;
}
