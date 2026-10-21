
#pragma once
#include <glad/gl.h>   // pour GLuint
#include <memory>
#include <vector>


/*

x = cos(θ) · sin(φ)
y = cos(φ)
z = sin(θ) · sin(φ)

le sommet de la ligne i et de la colonne j a pour indice i * n + j

*/
class Mesh {
public:
  // should properly set up the geometry buffer 
  void init();

  // should be called in the main rendering loop
  void render();

  // should properly delete the geometry buffer
  void clear();

  static std::shared_ptr<Mesh> genSphere(const int resolution=16);

private:
  std::vector<float> m_vertexPositions;
  std::vector<float> m_vertexNormals;
  std::vector<unsigned int> m_triangleIndices;
  std::vector<float> m_vertexTexCoords;
  GLuint m_vao = 0;
  GLuint m_posVbo = 0;
  GLuint m_normalVbo = 0;
  GLuint m_texCoordVbo = 0;
  GLuint m_ibo = 0;
};

/*
// All vertex positions packed in one array [x0, y0, z0, x1, y1, z1, ...]
std::vector<float> g_vertexPositions;
// All vertex colors packed in one array [r0, g0, b0, r1, g1, b1, ...]
std::vector<float> g_vertexColors;
// All triangle indices packed in one array [v00, v01, v02, v10, v11, v12, ...] with vij the index of j-th vertex of the i-th triangle
std::vector<unsigned int> g_triangleIndices;
*/
