#pragma once
#include "../Definition.h"
#include "../Transform.h"

class Camera {
public:
	enum class Type { Perspective, Orthographic };

public:
	Camera();
	
public:
	inline const Transform& GetTransform() const { return m_transform; }
	inline Transform& GetTransform() { return m_transform; }
	inline void SetPosition(const Vector3& pos) { m_transform.SetPosition(pos); }
	inline void SetRotation(const Quaternion& rot) { m_transform.SetRotation(rot); }
	inline void SetScale(const Vector3& scl) { m_transform.SetScale(scl); }

public:
	void SetPerspective(Scalar fovY, Scalar aspect, Scalar zNear, Scalar zFar);

	void SetOrthographic(Scalar left, Scalar right, Scalar bottom, Scalar top, Scalar zNear, Scalar zFar);

	const Matrix4& GetProjectionMatrix();

	const Matrix4& GetViewMatrix();

	Matrix4 GetMVPMatrix(const Transform& modelTransform);

	static void KeyBoardPressedListener(GLFWwindow* window, int key, int scancode, int action, int mods) {
		auto cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			constexpr Scalar step = 0.1f;
			switch (key) {
			case GLFW_KEY_W: cam->m_transform.Translate(Vector3(0.0, 0.0, -step)); break;
			case GLFW_KEY_S: cam->m_transform.Translate(Vector3(0.0, 0.0, step)); break;
			case GLFW_KEY_A: cam->m_transform.Translate(Vector3(-step, 0.0, 0.0)); break;
			case GLFW_KEY_D: cam->m_transform.Translate(Vector3(step, 0.0, 0.0)); break;
			case GLFW_KEY_Q: cam->m_transform.Translate(Vector3(0.0, -step, 0.0)); break;
			case GLFW_KEY_E: cam->m_transform.Translate(Vector3(0.0, step, 0.0)); break;
			}
			cam->m_viewDirty = true;
		}
	}

private:
	void ComputePerspectiveProjection();

	void ComputeOrthographicProjection();

private:
	Transform m_transform;
	Type m_type;
	Scalar m_fovY, m_aspect, m_zNear, m_zFar;
	Scalar m_left, m_right, m_bottom, m_top, m_zNearO, m_zFarO;

	Matrix4 m_viewMatrix;
	Matrix4 m_projMatrix;
	bool m_viewDirty;
	bool m_projDirty;
};