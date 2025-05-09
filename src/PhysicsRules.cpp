#include "PhysicsRules.h"


PhysicModel::PhysicModel(const std::string& filename) {
    json j = json::parse(std::ifstream(filename));
    m_damp = j.at("damping").get<Scalar>();
    m_collisionDist = j.at("collisionDistance").get<Scalar>();
    m_groundCollisionDist = j.at("groundCollisionDistance").get<Scalar>();
    m_groundCollisionMultiplier = j.at("groundCollisionMultiplier").get<Scalar>();

    for (Integer di = 0; di < 3; ++di) {
        m_gravity[di] = j.at("gravity")[di].get<Scalar>();
    }

    const auto& particles = j.at("Particles");
    m_ps.Resize(particles.size() + 1); // left index 0 empty for now
    Integer index = 0;
    for (const auto& particle : particles) {
        Particle<3>& p = m_ps.GetParticle(index);
        p.mass = particle.at("mass").get<Scalar>();
        p.isFixed = particle.at("isFixed").get<bool>();
        p.radius = particle.at("radius").get<Scalar>();

        for (Integer di = 0; di < 3; ++di) {
            p.position[di] = particle.at("position")[di].get<Scalar>();
            p.velocity[di] = particle.at("velocity")[di].get<Scalar>();
        }
        ++index;
    }

    const auto& springs = j.at("Springs");
    m_springs.resize(springs.size());
    index = 0;
    for (const auto& spring : springs) {
        Integer a = spring.at("a").get<Integer>();
        Integer b = spring.at("b").get<Integer>();
        Scalar stiffness = spring.at("stiffness").get<Scalar>();
        m_springs[index] = AddSpring(a, b, 1.0 / stiffness);
        ++index;
    }
}

Scalar PhysicModel::Remap01(Scalar inp, Scalar s, Scalar e) const {
    return MathUtils::Clamp((inp - s) / (e - s), 0.0f, 1.0f);
}

Integer PhysicModel::NearestParticle(const Vector3f& p) const {
    Integer best = 1;
    float   bestDist = std::numeric_limits<float>::infinity();
    for (Integer i = 0; i < m_ps.NumParticles(); ++i) {
        float d = (p - m_ps.GetPosition(i)).squaredNorm();
        if (d < bestDist) {
            bestDist = d;
            best = i;
        }
    }
    return best;
}

Spring PhysicModel::AddSpring(Integer a, Integer b, Scalar invK) {
    Spring s;
    s.a = a;
    s.b = b;
    s.restLength = (m_ps.GetPosition(a) - m_ps.GetPosition(b)).norm();
    s.invStiffness = invK;
    return s;
}

// --- Spring 约束 --- //
float PhysicModel::SpringConstraint(const Spring& s) const {
    return (m_ps.GetPosition(s.a) - m_ps.GetPosition(s.b)).norm()
        - s.restLength;
}

Vector3f PhysicModel::SpringConstraintGradient(const Vector3f& a,
    const Vector3f& b) const
{
    Vector3f diff = a - b;
    float n = diff.norm();
    diff /= n;
    return n > 0.0 ? diff : Vector3::Zero();
}

Vector3f PhysicModel::SpringConstraintGrad(const Spring& s,
    Integer particleIdx) const
{
    float sign = (particleIdx == s.a ? +1.0f : -1.0f);
    return sign * SpringConstraintGradient(
        m_ps.GetPosition(s.a),
        m_ps.GetPosition(s.b));
}

void PhysicModel::SolveSpring(const Spring& s, Scalar dt) {
    float numer = -SpringConstraint(s);
    auto  ga = SpringConstraintGrad(s, s.a);
    auto  gb = SpringConstraintGrad(s, s.b);

    float invMassA = MathUtils::IsSmall(m_ps.GetMass(s.a)) ? 0.0 : 1.0f / m_ps.GetMass(s.a);
    float invMassB = MathUtils::IsSmall(m_ps.GetMass(s.b)) ? 0.0 : 1.0f / m_ps.GetMass(s.b);
    float denom = invMassA * ga.squaredNorm() + invMassB * gb.squaredNorm();
    denom += s.invStiffness / (dt * dt);

    if (denom == 0.0f) return;
    float lambda = numer / denom;

    // PBD 位置更新
    m_ps.SetPosition(s.a,
        m_ps.GetPosition(s.a) + lambda * invMassA * ga);
    m_ps.SetPosition(s.b,
        m_ps.GetPosition(s.b) + lambda * invMassB * gb);
}

// --- Collision 约束 --- //
float PhysicModel::CollisionConstraint(const Vector3f& a,
    const Vector3f& b,
    Scalar cd) const
{
    float d = (a - b).norm();
    return (d < cd ? d - cd : 0.0f);
}

Vector3f PhysicModel::CollisionConstraintGradient(
    const Vector3f& a, const Vector3f& b, Scalar cd) const
{
    float d = (a - b).norm();
    if (d <= cd && d > 0) return (a - b) / d;
    return Vector3f::Zero();
}

void PhysicModel::SolveCollisionConstraint(Integer i, Integer j,
    Scalar cd, Scalar dt)
{
    auto& pi = m_ps.GetPosition(i), & pj = m_ps.GetPosition(j);
    float   numer = -CollisionConstraint(pi, pj, cd);
    Vector3f g = CollisionConstraintGradient(pi, pj, cd);

    float invMassA = MathUtils::IsSmall(m_ps.GetMass(i)) ? 0.0 : 1.0f / m_ps.GetMass(i);
    float invMassB = MathUtils::IsSmall(m_ps.GetMass(j)) ? 0.0 : 1.0f / m_ps.GetMass(j);
    float denom =
        invMassA * g.squaredNorm() +
        invMassB * g.squaredNorm() +
        (1.0f / 1000.0f) / (dt * dt);

    if (denom == 0) return;
    float lambda = numer / denom;
    m_ps.SetPosition(i, pi + lambda * invMassA * g);
    m_ps.SetPosition(j, pj - lambda * invMassB * g);
}

// --- Ground 约束 --- //
float PhysicModel::Phi(const Vector3f& p) const {
    constexpr float PI = 3.14159265359f;
    return p.y() - (0.1f * std::sin(2 * PI * p.x())
        * std::sin(2 * PI * p.z()) - 0.5f);
}

float PhysicModel::GroundConstraint(const Vector3f& p, Scalar gd) const {
    float phi = Phi(p);
    return (phi < gd ? phi - gd : 0.0f);
}

Vector3f PhysicModel::GroundConstraintGradient(const Vector3f& p,
    Scalar gd) const
{
    constexpr float PI = 3.14159265359f;
    float phi = Phi(p);
    if (phi < gd) {
        float gx = -0.1f * 2 * PI * std::cos(2 * PI * p.x()) * std::sin(2 * PI * p.z());
        float gz = -0.1f * 2 * PI * std::cos(2 * PI * p.z()) * std::sin(2 * PI * p.x());
        return Vector3(gx, 1.0f, gz) * m_groundCollisionMultiplier;
    }
    return Vector3f::Zero();
}

void PhysicModel::SolveGroundConstraint(Integer i, Scalar gd, Scalar dt) {
    float numer = -GroundConstraint(m_ps.GetPosition(i), gd);
    Vector3f g = GroundConstraintGradient(m_ps.GetPosition(i), gd);
    float invMass = MathUtils::IsSmall(m_ps.GetMass(i)) ? 0.0 : 1.0f / m_ps.GetMass(i);
    float denom = invMass * g.squaredNorm() + (1.0f / 1000.0f) / (dt * dt);
    if (denom == 0) return;
    float lambda = numer / denom;
    m_ps.SetPosition(i, m_ps.GetPosition(i) + lambda * invMass * g);
}

// —— 全部约束一次 solve —— //
void PhysicModel::SolveConstraints(Scalar dt) {
    for (size_t i = 1; i < m_springs.size(); ++i)
        SolveSpring(m_springs[i], dt);

    for (Integer i = 1; i < m_ps.NumParticles(); ++i)
        SolveGroundConstraint(i, m_groundCollisionDist, dt);

    // no between particle collision now
}

float PhysicModel::DistToSegment(const Vector3f& p,
    const Vector3f& a,
    const Vector3f& b) const
{
    Vector3f pa = p - a, ba = b - a;
    float h = MathUtils::Clamp(pa.dot(ba) / ba.dot(ba), 0.0f, 1.0f);
    return (pa - h * ba).norm();
}

void PhysicModel::Step(const Scalar& dt) {
    for (int i = 0; i < 5; i++) {
        // Update rope particles only; skip updating the mouse particle since it's fixed.
        for (int j = 0; j < m_ps.NumParticles(); j++) {
            Particle<3>& particle = m_ps.GetParticle(j);
            if (!particle.isFixed) particle.velocity += dt * m_gravity;

            particle.velocity *= exp(-m_damp * dt);
            particle.prevPosition = particle.position;
            particle.position += dt * particle.velocity;
        }
        SolveConstraints(dt);

        for (int j = 0; j < m_ps.NumParticles(); j++) {
            Particle<3>& particle = m_ps.GetParticle(j);
            if (!particle.isFixed) {
                particle.velocity = (particle.position - particle.prevPosition) / dt;
            }
        }
    }
}