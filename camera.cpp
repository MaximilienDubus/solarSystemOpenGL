#include "camera.h"


float Camera::getFov() const { return m_fov; }
void Camera::setFoV(const float f) { m_fov = f; }
float Camera::getAspectRatio() const { return m_aspectRatio; }
void Camera::setAspectRatio(const float a) { m_aspectRatio = a; }
float Camera::getNear() const { return m_near; }
void Camera::setNear(const float n) { m_near = n; }
float Camera::getFar() const { return m_far; }
void Camera::setFar(const float n) { m_far = n; }
void Camera::setPosition(const glm::vec3 &p) { m_pos = p; }
glm::vec3 Camera::getPosition() const { return m_pos; }

// from world space to camera space
glm::mat4 Camera::computeViewMatrix() const {
    return glm::lookAt(m_pos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
}

// from camera space to clip space
// Returns the projection matrix stemming from the camera intrinsic parameter.
glm::mat4 Camera::computeProjectionMatrix() const {
    return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_near, m_far);
}
