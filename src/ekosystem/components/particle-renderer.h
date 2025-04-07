#pragma once

#include <suprengine/components/renderer.h>

#include <suprengine/math/vec3.h>

#include <suprengine/rendering/texture.h>

#include <vector>

namespace eks
{
	using namespace suprengine;

	struct ParticleSystemData
	{
		SafePtr<Texture> texture = nullptr;
		Vec3 render_scale = Vec3::one;

		//	Starting velocity, in world space, for a new particle.
		Vec3 start_velocity = Vec3::zero;
		//	Velocity, in world space, to apply per second.
		Vec3 frame_velocity = Vec3::zero;

		Vec3 spawn_location_offset = Vec3::zero;

		//	Amount of particles to spawn per second.
		float spawn_rate = 1.0f;
		//	Maximum time in seconds for a particle to live.
		float max_lifetime = 5.0f;
	};

	struct Particle
	{
		Vec3 location = Vec3::zero;
		Vec3 velocity = Vec3::zero;

		float lifetime = 0.0f;
	};

	class ParticleRenderer : public Renderer
	{
	public:
		void update( float dt ) override;
		void render( RenderBatch* render_batch ) override;

		RenderPhase get_render_phase() const override;

	public:
		ParticleSystemData system_data {};

	private:
		void _create_particle();

	private:
		std::vector<Particle> _particles {};
		float _spawn_time = 0.0f;
	};
}