#pragma once

#include <suprengine/components/renderer.h>

#include <suprengine/math/vec3.h>

#include <suprengine/rendering/texture.h>

#include <vector>

namespace eks
{
	using namespace suprengine;

	/*
	 * Structure representing a single particle instance from the ParticleRenderer.
	 */
	struct ParticleInstance
	{
	public:
		//	Unique index for this particle, ranging from 0 to 65535.
		uint16 unique_id = 0;

		//	Location of the particle.
		Vec3 location = Vec3::zero;
		//	Offset to apply on the location for rendering.
		Vec3 offset = Vec3::zero;
		//	Velocity to apply on the particle's location per second.
		Vec3 velocity = Vec3::zero;

		//	Scale of the particle for rendering.
		Vec3 scale = Vec3::one;
		//	Color to modulate the particle's rendering with.
		Color modulate = Color::white;

		float lifetime = 0.0f;
	};

	/*
	 * Structure representing the template specifying how particles should
	 * behave and render.
	 */
	struct ParticleSystemData
	{
		SafePtr<Texture> texture = nullptr;
		SafePtr<Shader> shader = nullptr;
		Mesh* mesh = nullptr;

		Vec3 render_scale = Vec3::one;

		//	Starting velocity, in world space, for a new particle.
		Vec3 start_velocity = Vec3::zero;
		//	Velocity, in world space, to apply per second.
		Vec3 frame_velocity = Vec3::zero;

		Vec3 spawn_location_offset = Vec3::zero;

		std::function<void( ParticleInstance*, int, float )> custom_updater = nullptr;

		//	Amount of particles to spawn per second.
		float spawn_rate = 1.0f;
		//	Maximum time in seconds for a particle to live.
		float max_lifetime = 5.0f;
	};

	class ParticleRenderer : public Renderer
	{
	public:
		ParticleRenderer( 
			Color modulate = Color::white, 
			int priority_order = -10
		);

		void update( float dt ) override;
		void render( RenderBatch* render_batch ) override;

		RenderPhase get_render_phase() const override;

	public:
		ParticleSystemData system_data {};
		bool is_spawning = true;
		float play_rate = 1.0f;

	private:
		void _spawn_particle();

	private:
		std::vector<ParticleInstance> _particles {};
		uint16 _next_unique_id = 0;
		float _spawn_time = 0.0f;
	};
}