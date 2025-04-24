#pragma once

#include "Definition.h"

template <int DIM>
struct Particle {
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	VectorX<DIM> position;
	VectorX<DIM> prevPosition;
	VectorX<DIM> velocity;
	Scalar mass;
	bool isFixed;

	Particle()
		: position(VectorX<DIM>::Zero()), 
		prevPosition(VectorX<DIM>::Zero()),
		velocity(VectorX<DIM>::Zero()),mass(1.0), isFixed(false) { }
};

template <int DIM>
class ParticleSystem {
public:
	ParticleSystem(Integer numParticles);

	inline const std::vector<Particle<DIM>>& GetParticles() const { return m_particle; }

	inline const VectorX<DIM>& GetPosition(Integer index) const { return m_particle[index].position; }
	inline const VectorX<DIM>& GetPrevPosition(Integer index) const { return m_particle[index].position; }
	inline const VectorX<DIM>& GetVelocity(Integer index) const { return m_particle[index].velocity; }
	inline const Particle<DIM>& GetParticle(Integer index) const { return m_particle[index]; }
	inline const Scalar& GetMass(Integer index) const { return m_particle[index].isFixed ? 0.0 : m_particle[index].mass; }
	inline Particle<DIM>& GetParticle(Integer index) { return m_particle[index]; }
	inline bool IsFixed(Integer index) { return m_particle[index].isFixed; }

	inline void SetPosition(Integer index, const VectorX<DIM>& position) { m_particle[index].position = position; }
	inline void SetPrevPosition(Integer index, const VectorX<DIM>& prevPosition) { m_particle[index].prevPosition = prevPosition; }
	inline void SetVelocity(Integer index, const VectorX<DIM>& velocity) { m_particle[index].velocity = velocity; }
	inline void SetMass(Integer index, const Scalar& val) { m_particle[index].mass = val; }
	inline void SetIsFixed(Integer index, bool val) { m_particle[index].isFixed = val; }

	inline const Integer& NumParticles() const { return m_size; }

	void Resize(Integer size);

	void WriteToFile(const std::string& filename, bool isAppend = false) const;
	void LoadFromLine(const std::string& line);

private:
	Integer m_size;
	std::vector<Particle<DIM>> m_particle;
};

