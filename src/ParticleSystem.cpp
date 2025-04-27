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
void ParticleSystem<DIM>::WriteToFile(const std::string& filename) const {
	static_assert(DIM == 2 or DIM == 3);

	std::filesystem::path p(filename);
	std::filesystem::path parent = p.parent_path();
	if (!parent.empty() && !std::filesystem::exists(parent)) {
		std::filesystem::create_directories(parent);
	}

	std::ofstream writer;
	writer.open(filename, std::ios::out | std::ios::trunc);

	if (!writer.is_open()) {
		throw std::runtime_error("Failed to open file: " + filename);
	}

	writer << "ply\nformat ascii 1.0\n";
	writer << "element vertex " << m_particle.size() << "\n";
	writer << "property float x\nproperty float y\nproperty float z\n";
	writer << "end_header\n";

	for (const Particle<DIM>& particle : m_particle) {
		Scalar z = 0.0;
		if constexpr (DIM == 3) z = particle.position.z();
		writer << particle.position.x() << " " << particle.position.y() << " " << z << "\n";
	}
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
