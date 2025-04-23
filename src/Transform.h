#pragma once

#include "Definition.h"


struct Transform {
    Affine3 tf = Affine3::Identity();

    Transform() = default;

    Transform(const Vector3& pos, const Quaternion& rot, const Vector3& scl) {
        tf = Eigen::Translation<Scalar, 3>(pos) * rot * Eigen::Scaling(scl);
    }

    inline void SetPosition(const Vector3& p) { 
        tf.prerotate(Quaternion::Identity()); 
        tf.pretranslate(p - tf.translation()); 
    }
    inline Vector3 GetPosition() const { return tf.translation(); }

    inline void SetRotation(const Quaternion& r) {
        Vector3 s = GetScale();
        tf = Eigen::Translation<Scalar, 3>(GetPosition())
            * r
            * Eigen::Scaling(s);
    }
    inline Quaternion GetRotation() const { return Quaternion(tf.rotation()); }

    inline void SetScale(const Vector3& s) {
        tf = Eigen::Translation<Scalar, 3>(GetPosition()) * GetRotation() * Eigen::Scaling(s);
    }
    inline Vector3 GetScale() const {
        Matrix3 R = tf.rotation().matrix();
        return Vector3(R.col(0).norm(), R.col(1).norm(), R.col(2).norm());
    }

    inline Matrix4 GetMatrix() const {
        return tf.matrix();
    }

    inline Matrix4 GetInverseMatrix() const {
        return tf.inverse().matrix();
    }

	inline void Translate(const Vector3& t) {
		tf.translate(t);
	}

	inline void Rotate(const Quaternion& r) {
		tf.rotate(r);
	}

	inline void Scale(const Vector3& s) {
		tf.scale(s);
	}   
};
