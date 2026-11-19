#include "../headers/ParticleSystem.hpp"

#include <cmath>
#include <random>
#include <vector>

namespace
{
	std::vector<Particle> CreateInitialParticles(std::uint32_t count)
	{
		std::mt19937 rng(1337);
		std::normal_distribution<float> normal(0.0f, 1.0f);
		std::uniform_real_distribution<float> uniform01(0.0f, 1.0f);
		std::uniform_real_distribution<float> lifetime(2.0f, 6.0f);

		constexpr float radius = 1.5f;

		std::vector<Particle> particles(count);
		for (Particle& particle : particles)
		{
			//Normalised gaussian = uniform direction
			//cbrt gives uniform density inside the sphere
			const float x = normal(rng);
			const float y = normal(rng);
			const float z = normal(rng);
			const glm::vec3 direction = glm::normalize(glm::vec3(x, y, z));
			const float distance = radius * std::cbrt(uniform01(rng));

			particle.position = direction * distance;
			particle.age = 0.0f;
			particle.velocity = glm::vec3(0.0f);
			particle.lifetime = lifetime(rng);
			particle.color = glm::vec4(0.5f + 0.5f * direction, 1.0f);
		}
		return particles;
	}
}

ParticleSystem::ParticleSystem(std::uint32_t particleCount)
	: m_count(particleCount)
{
	const std::vector<Particle> initial = CreateInitialParticles(particleCount);

	glGenBuffers(1, &m_buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
	glBufferData(
		GL_SHADER_STORAGE_BUFFER,
		static_cast<GLsizeiptr>(initial.size() * sizeof(Particle)),
		initial.data(),
		GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

ParticleSystem::~ParticleSystem()
{
	glDeleteBuffers(1, &m_buffer);
}