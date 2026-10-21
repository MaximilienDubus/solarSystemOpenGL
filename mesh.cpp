
#define _USE_MATH_DEFINES

#include "mesh.h"
#include <iostream>
#include <cmath>



// should properly set up the geometry buffer 
void Mesh::init(){
    // Create a single handle, vertex array object that contains attributes,
    // vertex buffer objects (e.g., vertex's position, normal, and color)
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // Generate a GPU buffer to store the positions of the vertices
    size_t vertexBufferSize = sizeof(float)*m_vertexPositions.size(); // Gather the size of the buffer from the CPU-side vector
    glGenBuffers(1, &m_posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, m_vertexPositions.data(), GL_DYNAMIC_READ);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    // Generate a GPU buffer to store the normals of the vertices
    size_t normalBufferSize = sizeof(float)*m_vertexNormals.size(); // Gather the size of the buffer from the CPU-side vector
    glGenBuffers(1, &m_normalVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_normalVbo);
    glBufferData(GL_ARRAY_BUFFER, normalBufferSize, m_vertexNormals.data(), GL_DYNAMIC_READ);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1);

    // Generate a GPU buffer to store the texture of the vertices
    size_t texCoordBufferSize = sizeof(float)*m_vertexTexCoords.size(); // Gather the size of the buffer from the CPU-side vector
    glGenBuffers(1, &m_texCoordVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_texCoordVbo);
    glBufferData(GL_ARRAY_BUFFER, texCoordBufferSize, m_vertexTexCoords.data(), GL_DYNAMIC_READ);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);
    glEnableVertexAttribArray(2);

    // Same for an index buffer object that stores the list of indices of the
    // triangles forming the mesh
    size_t indexBufferSize = sizeof(unsigned int)*m_triangleIndices.size();

    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, m_triangleIndices.data(), GL_DYNAMIC_READ);

    glBindVertexArray(0); // deactivate the VAO for now, will be activated again when rendering
}

  

// should be called in the main rendering loop
void Mesh::render(){
    glBindVertexArray(m_vao);     // activate the VAO storing geometry data
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_triangleIndices.size()), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program
}


std::shared_ptr<Mesh> Mesh::genSphere(const int resolution) {

    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

    // on a line (eg. Equator) the first and the last point overlap
    // on a column (eg. Greenwich meridian) the first point is the North pole and the last is the South pole.
    // there exists resolution + 1 South pole that overlap, same for north pole

    // for every vertex
    for (int phi = 0; phi < resolution +1; phi++) {  // resolution + 1 points in a column
        for (int theta = 0; theta < resolution +1; theta++) {  // resolution + 1 points in a line
        float normalised_phi = ((float)phi / (float)resolution) * (float)M_PI; // 0 included to pi included
        float normalised_theta = ((float)theta / (float)resolution) * 2 *  (float)M_PI; // 0 included to 2pi included

        // the three coordinates difining a point
        float x = std::cos(normalised_theta) * std::sin(normalised_phi);
        float y = std::cos(normalised_phi);
        float z = std::sin(normalised_theta) * std::sin(normalised_phi);

        // adding the point
        mesh->m_vertexPositions.push_back(x);
        mesh->m_vertexPositions.push_back(y);
        mesh->m_vertexPositions.push_back(z);

        // for a sphere the normal is the vertex
        mesh->m_vertexNormals.push_back(x);
        mesh->m_vertexNormals.push_back(y);
        mesh->m_vertexNormals.push_back(z);

        // texture coordinate are normalised theta and phi
        float u = 1.0f - normalised_theta / (2.0f * (float)M_PI);
        float v = normalised_phi / (float)M_PI;
        mesh->m_vertexTexCoords.push_back(u); 
        mesh->m_vertexTexCoords.push_back(v); 
        }
    }

    unsigned int n = resolution + 1; // number of vertex on a line

    // for every face
    for (int i = 0; i < resolution; i++) {
        for (int j = 0; j < resolution; j++) {

        unsigned int i_j = i*n + j;
        unsigned int i_j1 = i*n + j + 1;
        unsigned int i1_j = (i+1)*n + j;
        unsigned int i1_j1 = (i+1)*n + j + 1;

        //there are two triangles to add for each face :
        // (i, j) - (i, j+1) - (i+1, j)
        // (i, j+1) - (i+1, j+1) - (i+1, j)

        mesh->m_triangleIndices.push_back(i_j); // adding the three points that make a triangle
        mesh->m_triangleIndices.push_back(i_j1);
        mesh->m_triangleIndices.push_back(i1_j);

        mesh->m_triangleIndices.push_back(i_j1);
        mesh->m_triangleIndices.push_back(i1_j1);
        mesh->m_triangleIndices.push_back(i1_j);
        }
    }

    return mesh;
}

void Mesh::clear() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_posVbo);
    glDeleteBuffers(1, &m_normalVbo);
    glDeleteBuffers(1, &m_texCoordVbo);
    glDeleteBuffers(1, &m_ibo);
}
