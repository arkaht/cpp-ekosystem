#include "particle-renderer.h"

#include <suprengine/core/engine.h>
#include <suprengine/tools/vis-debug.h>

using namespace eks;

void ParticleRenderer::update( float dt )
{
	//	Spawn new particles with spawn rate
	_spawn_time -= dt;
	if ( _spawn_time < 0.0f )
	{
		_create_particle();

		const float next_spawn_time = 1.0f / system_data.spawn_rate;
		_spawn_time += next_spawn_time;
	}

	//	Update all particles
	for ( auto itr = _particles.begin(); itr != _particles.end(); )
	{
		Particle& particle = *itr;

		//	Update lifetime and check for death
		particle.lifetime += dt;
		if ( particle.lifetime > system_data.max_lifetime )
		{
			itr = _particles.erase( itr );
			continue;
		}

		particle.location += particle.velocity;
		particle.velocity += system_data.frame_velocity * dt;

		itr++;
	}
}

void ParticleRenderer::render( RenderBatch* render_batch )
{
	if ( _particles.empty() ) return;

	SharedPtr<Texture> texture = system_data.texture.lock();
	ASSERT( texture != nullptr );

	Engine& engine = Engine::instance();
	Quaternion camera_rotation = Quaternion::look_at( -engine.camera->transform->get_up(), -engine.camera->transform->get_forward() );
	camera_rotation = Quaternion::concatenate( camera_rotation, Quaternion( engine.camera->transform->get_forward(), math::HALF_PI ) );

	for ( const Particle& particle : _particles )
	{
		const Mtx4 matrix = Mtx4::create_from_transform( transform->scale * system_data.render_scale, camera_rotation, particle.location );
		render_batch->draw_texture(
			matrix,
			texture,
			Vec2::one * 0.5f,
			Rect { Vec2::zero, system_data.texture->get_size() },
			Color::white
		);

	#ifdef ENABLE_VISDEBUG
		VisDebug::add_sphere( particle.location, ( transform->scale * system_data.render_scale ).length() / 5.0f, Color::white, 0.0f );
	#endif
	}
}

RenderPhase ParticleRenderer::get_render_phase() const
{
	return RenderPhase::World;
}

void ParticleRenderer::_create_particle()
{
	Particle particle {};
	particle.location = transform->location + system_data.spawn_location_offset;
	particle.velocity = system_data.start_velocity;
	particle.lifetime = 0.0f;
	_particles.push_back( particle );
}
