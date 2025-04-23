#include "Camera.h"

Camera::Camera() 
	: m_type(Type::Perspective), 
	m_fovY(45.0f), m_aspect(1.0f), m_zNear(0.1f), m_zFar(100.0f),
	m_left(-1.0f), m_right(1.0f), m_bottom(-1.0f), m_top(1.0f), m_zNearO(0.1f), m_zFarO(100.0f),
	m_viewDirty(true), m_projDirty(true) {

	SetPerspective(m_fovY, m_aspect, m_zNear, m_zFar);
}

void Camera::SetPerspective(Scalar fovY, Scalar aspect, Scalar zNear, Scalar zFar) {
	m_type = Type::Perspective;
	m_fovY = fovY;
	m_aspect = aspect;
	m_zNear = zNear;
	m_zFar = zFar;
	m_projDirty = true;
}

void Camera::SetOrthographic(Scalar left, Scalar right, Scalar bottom, Scalar top, Scalar zNear, Scalar zFar) {
	m_type = Type::Orthographic;
	m_left = left;
	m_right = right;
	m_bottom = bottom;
	m_top = top;
	m_zNearO = zNear;
	m_zFarO = zFar;
	m_projDirty = true;
}

const Matrix4& Camera::GetViewMatrix() {
	if (m_viewDirty) {
		m_viewMatrix = m_transform.GetInverseMatrix();
		m_viewDirty = false;
	}
	return m_viewMatrix;
}

const Matrix4& Camera::GetProjectionMatrix() {
	if (m_projDirty) {
		if (m_type == Type::Perspective) {
			ComputePerspectiveProjection();
		}
		else {
			ComputeOrthographicProjection();
		}
		m_projDirty = false;
	}
	return m_projMatrix;
}

Matrix4 Camera::GetMVPMatrix(const Transform& modelTransform) {
	Matrix4 ans = GetProjectionMatrix() * GetViewMatrix() * modelTransform.GetMatrix();
	return ans;
}

void Camera::ComputePerspectiveProjection() {
	Scalar f = Scalar(1) / std::tan(m_fovY * Scalar(0.5) * PI / Scalar(180));
	m_projMatrix.setZero();
	m_projMatrix(0, 0) = f / m_aspect;
	m_projMatrix(1, 1) = f;
	m_projMatrix(2, 2) = (m_zFar + m_zNear) / (m_zNear - m_zFar);
	m_projMatrix(2, 3) = (Scalar(2) * m_zFar * m_zNear) / (m_zNear - m_zFar);
	m_projMatrix(3, 2) = -Scalar(1);
}

void Camera::ComputeOrthographicProjection() {
	m_projMatrix.setZero();
	m_projMatrix(0, 0) = Scalar(2) / (m_right - m_left);
	m_projMatrix(1, 1) = Scalar(2) / (m_top - m_bottom);
	m_projMatrix(2, 2) = Scalar(-2) / (m_zFar - m_zNear);
	m_projMatrix(0, 3) = -(m_right + m_left) / (m_right - m_left);
	m_projMatrix(1, 3) = -(m_top + m_bottom) / (m_top - m_bottom);
	m_projMatrix(2, 3) = -(m_zFar + m_zNear) / (m_zFar - m_zNear);
	m_projMatrix(3, 3) = Scalar(1);
}
