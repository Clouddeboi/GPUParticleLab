#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <cstddef>
#include <cstdint>

struct Particle
{
	glm::vec3 position;
	float age;
	glm::vec3 velocity;
	float lifetime;
	glm::vec4 color;
};

static_assert(sizeof(Particle) == 48, "Particle must match the std40 layout (48 bytes");
static_assert(offsetof(Particle, age) == 12, "Particle::age offset mismatch");
static_assert(offsetof(Particle, velocity) == 16, "Particle::velocity offset mismatch");
static_assert(offsetof(Particle, lifetime) == 28, "Particle::lifetime offset mismatch");
static_assert(offsetof(Particle, color) == 32, "Particle::color offset mismatch");

class ParticleSystem
{
public:
	explicit ParticleSystem(std::uint32_t particleCount);
	~ParticleSystem();

	ParticleSystem(const ParticleSystem&) = delete;
	ParticleSystem& operator = (const ParticleSystem&) = delete;

	GLuint GetBuffer() const { return m_buffer; }
	std::uint32_t GetCount() const { return m_count; }

private:
	GLuint m_buffer;
	std::uint32_t m_count = 0;
};