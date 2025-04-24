
#pragma once
#include "Definition.h"
#include "MathUtils.h"
#include "ParticleSystem.h"

struct Spring {
    Integer a, b;
    Scalar  restLength;
    Scalar  invStiffness;
};

class PhysicModel {
public:
    PhysicModel();

    Scalar Remap01(Scalar inp, Scalar start, Scalar end) const;
    Integer NearestParticle(const Vector3f& p) const;
    Spring AddSpring(Integer a, Integer b, Scalar invK);

    float SpringConstraint(const Spring& s) const;
    Vector3f SpringConstraintGradient(const Vector3f& a, const Vector3f& b) const;
    Vector3f SpringConstraintGrad(const Spring& s, Integer particleIdx) const;
    void SolveSpring(const Spring& s, Scalar dt);

    float CollisionConstraint(const Vector3f& a,const Vector3f& b,Scalar collisionDist) const;
    Vector3f CollisionConstraintGradient(const Vector3f& a, const Vector3f& b, Scalar collisionDist) const;
    void SolveCollisionConstraint(Integer i, Integer j, Scalar collisionDist, Scalar dt);

    float GroundConstraint(const Vector3f& p, Scalar groundDist) const;
    Vector3f GroundConstraintGradient(const Vector3f& p, Scalar groundDist) const;
    void SolveGroundConstraint(Integer i, Scalar groundDist, Scalar dt);

    void SolveConstraints(Scalar dt);

    float Phi(const Vector3f& p) const;
    float DistToSegment(const Vector3f& p, const Vector3f& a, const Vector3f& b) const;

    void Step(const Scalar& dt);

    inline ParticleSystem<3>& GetParticleSystem() { return m_ps; }

private:
    ParticleSystem<3> m_ps;
    std::vector<Spring> m_springs;
    Scalar m_damp;
    Scalar m_collisionDist;
    Scalar m_groundCollisionDist;
    Vector3f m_gravity;
};
