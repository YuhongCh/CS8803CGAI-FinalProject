#version 450 core

layout(location = 0) in vec3 vertexPos;
uniform float pointSize;
uniform mat4 mvpMatrix;

void main() {
    gl_Position = mvpMatrix * vec4(vertexPos, 1.0);
    gl_PointSize = pointSize;
}
