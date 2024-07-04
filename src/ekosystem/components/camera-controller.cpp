#include "camera-controller.h"

#include <suprengine/entity.h>

using namespace eks;

CameraController::CameraController(
	float move_speed,
	const Vec3& offset
)
	: move_speed( move_speed ),
	  offset( offset )
{}

void CameraController::setup()
{
	transform->location += offset;
}

void CameraController::update( float dt )
{
	auto& engine = Engine::instance();
	auto inputs = engine.get_inputs();

	//  Don't use scaled delta time (for dev. menu)
	dt = engine.get_updater()->get_unscaled_delta_time();

	//  Retrieve input direction
	Vec3 dir {};
	dir.y = inputs->get_keys_as_axis( SDL_SCANCODE_S, SDL_SCANCODE_W );
	dir.x = inputs->get_keys_as_axis( SDL_SCANCODE_A, SDL_SCANCODE_D );
	dir.normalize2d();

	Vec3 pos = transform->location;
	if ( dir == Vec3::zero )
	{
		if ( focus_target.is_valid() )
		{
			pos = focus_target->location + offset;
		}
	}
	else
	{
		//  Break focus target reference
		focus_target.reset();

		//  Flatten forward (get rid of camera's pitch)
		Vec3 forward = transform->get_forward();
		forward.normalize2d();

		Vec3 right = transform->get_right();

		//  Transform direction according to camera's orientation
		dir = dir.x * right 
			+ dir.y * forward;

		//  Compute final movement vector 
		Vec3 movement = dir * ( move_speed * dt );
		pos = transform->location + movement;
	}

	//  Apply new location
	transform->set_location( pos );
}
