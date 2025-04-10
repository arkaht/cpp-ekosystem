#include "particle-renderer.h"

#include <suprengine/core/engine.h>
#include <suprengine/tools/vis-debug.h>
#include <suprengine/rendering/shader.h>

#include <gl/glew.h>

using namespace eks;

float Particle::get_lifetime() const
{
	float game_time = Engine::instance().get_updater()->get_accumulated_seconds();
	return game_time - start_game_time;
}


ParticleRenderer::ParticleRenderer( Color modulate, int priority_order )
	: Renderer( modulate, priority_order )
{}

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
	int index = 0;
	float game_time = Engine::instance().get_updater()->get_accumulated_seconds();
	for ( auto itr = _particles.begin(); itr != _particles.end(); )
	{
		Particle& particle = *itr;

		//	Update lifetime and check for death
		if ( particle.get_lifetime() > system_data.max_lifetime )
		{
			itr = _particles.erase( itr );
			continue;
		}

		particle.location += particle.velocity * dt;
		particle.velocity += system_data.frame_velocity * dt;

		if ( system_data.custom_updater != nullptr )
		{
			system_data.custom_updater( &particle, index, dt );
		}

		itr++;
		index++;
	}
}

void ParticleRenderer::render( RenderBatch* render_batch )
{
	if ( _particles.empty() ) return;

	Mesh* mesh = system_data.mesh;
	ASSERT( mesh != nullptr );

	SharedPtr<Shader> shader = system_data.shader.lock();
	ASSERT( shader != nullptr );

	SharedPtr<Texture> texture = system_data.texture.lock();
	ASSERT( texture != nullptr );

	Engine& engine = Engine::instance();
	const Vec3 look_at_direction = engine.camera->transform->get_forward();
	Quaternion camera_rotation = Quaternion::look_at( look_at_direction, Vec3::up );
	camera_rotation = Quaternion::concatenate( camera_rotation, Quaternion( look_at_direction, math::HALF_PI ) );

	const Vec2 size = texture->get_size();
	shader->activate();
	shader->set_vec4( "u_source_rect", 0.0f, 0.0f, 1.0f, 1.0f );
	//shader->set_vec2( "u_origin", Vec2 { 0.5f, 0.5f } );

	glDisable( GL_CULL_FACE );
	for ( const Particle& particle : _particles )
	{
		const Mtx4 matrix = Mtx4::create_from_transform(
			transform->scale * system_data.render_scale * particle.scale,
			camera_rotation,
			particle.location + particle.offset
		);
		render_batch->draw_mesh( matrix, mesh, shader, texture, Color::white );

	#ifdef ENABLE_VISDEBUG
		VisDebug::add_sphere( particle.location, ( transform->scale * system_data.render_scale ).length(), particle.modulate, 0.0f );
	#endif
	}
}

RenderPhase ParticleRenderer::get_render_phase() const
{
	return RenderPhase::World;
}

void ParticleRenderer::_create_particle()
{
	float game_time = Engine::instance().get_updater()->get_accumulated_seconds();

	Particle particle {};
	particle.location = transform->location + system_data.spawn_location_offset;
	particle.velocity = system_data.start_velocity;
	particle.start_game_time = game_time;
	_particles.push_back( particle );
}
