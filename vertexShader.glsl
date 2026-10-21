#version 330 core            // Minimal GL version support expected from the GPU

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoord;
uniform mat4 worldMat, viewMat, projMat;

out vec3 fNormal;
out vec3 fWorldPosition;
out vec2 fTexCoord;

void main() {
        vec4 worldPos = worldMat * vec4(vPosition, 1.0);
        gl_Position = projMat * viewMat * worldPos;
        fWorldPosition = worldPos.xyz;
        fNormal = mat3(transpose(inverse(worldMat))) * vNormal;
        fTexCoord = vTexCoord;
}
