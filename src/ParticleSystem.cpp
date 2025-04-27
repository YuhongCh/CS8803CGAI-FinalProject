#include "ParticleSystem.h"

template class ParticleSystem<2>;
template class ParticleSystem<3>;

template<int DIM>
ParticleSystem<DIM>::ParticleSystem(Integer numParticles)
	: m_size(numParticles), m_particle(numParticles){}

template<int DIM>
void ParticleSystem<DIM>::Resize(Integer size) {
	m_particle.resize(size);
	m_size = size;
}

template<int DIM>
void ParticleSystem<DIM>::WriteToFile(const std::string& filename, bool isAppend) const {
	std::ofstream writer;
	if (isAppend) writer.open(filename, std::ios::out | std::ios::app);
	else writer.open(filename, std::ios::out | std::ios::trunc);

	if (!writer.is_open()) {
		throw std::runtime_error("Failed to open file: " + filename);
	}

	writer << m_particle.size() << "\n";
	for (const Particle<DIM>& particle : m_particle) {
		for (Integer di = 0; di < DIM; ++di) {
			writer << particle.position[di] << ",";
		}
	}
	writer << "\n";
	writer.close();
}


template<int DIM>
void ParticleSystem<DIM>::LoadFromLine(const std::string& line) {
	std::string token;
	std::istringstream ss(line);

	std::getline(ss, token, ',');
	for (size_t i = 0; i < m_size; ++i) {
		for (Integer di = 0; di < DIM; ++di) {
			if (!std::getline(ss, token, ',')) {
				continue;
				throw std::runtime_error("Failed to recognize line " + line);
			}
			m_particle[i].position[di] = std::stof(token);
		}
	}
}
