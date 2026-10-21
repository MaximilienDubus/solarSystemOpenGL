
#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>


// Basic camera model
class Camera {
public:
  float getFov() const;
  void setFoV(const float f);
  float getAspectRatio() const ;
  void setAspectRatio(const float a);
  float getNear() const ;
  void setNear(const float n);
  float getFar() const;
  void setFar(const float n);
  void setPosition(const glm::vec3 &p);
  glm::vec3 getPosition() const;

  // from world space to camera space
  glm::mat4 computeViewMatrix() const;

  // from camera space to clip space
  // Returns the projection matrix stemming from the camera intrinsic parameter.
  glm::mat4 computeProjectionMatrix() const ;

private:
  glm::vec3 m_pos = glm::vec3(0, 0, 0);
  float m_fov = 45.f;        // Field of view, in degrees
  float m_aspectRatio = 1.f; // Ratio between the width and the height of the image
  float m_near = 0.1f; // Distance before which geometry is excluded from the rasterization process
  float m_far = 10.f; // Distance after which the geometry is excluded from the rasterization process
};