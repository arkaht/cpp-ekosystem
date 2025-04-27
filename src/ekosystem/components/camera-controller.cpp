#include "camera-controller.h"

#include <suprengine/core/engine.h>
#include <suprengine/core/entity.h>

using namespace eks;

CameraController::CameraController(
	float move_speed,
	const Vec3& offset
)
	: move_speed( move_speed ), camera_offset( offset )
{}

void CameraController::setup()
{
	transform->set_location( start_location );
	transform->set_rotation( camera_rotation );
}

void CameraController::update( float dt )
{
	auto& engine = Engine::instance();
	auto inputs = engine.get_inputs();

	// Don't use scaled delta time (for dev. menu)
	dt = engine.get_updater()->get_unscaled_delta_time();

	// Retrieve input direction
	Vec3 dir {};
	dir.y = inputs->get_keys_as_axis( SDL_SCANCODE_S, SDL_SCANCODE_W );
	dir.x = inputs->get_keys_as_axis( SDL_SCANCODE_A, SDL_SCANCODE_D );

	// TODO: Move the editor camera code inside the engine
	static bool is_editor_camera = false;
	static float editor_camera_speed = 1.0f;
	
	if ( !inputs->is_key_down( SDL_SCANCODE_LSHIFT ) &&inputs->is_key_just_pressed( SDL_SCANCODE_F1 ) )
	{
		is_editor_camera = !is_editor_camera;

		if ( is_editor_camera )
		{
			transform->set_location( transform->location + engine.camera->get_offset() );
			engine.camera->set_offset( Vec3::zero );

			inputs->set_relative_mouse_mode( true );
		}
		else
		{
			transform->set_location( start_location );
			transform->set_rotation( camera_rotation );

			inputs->set_relative_mouse_mode( false );
		}
	}

	if ( is_editor_camera )
	{
		if ( inputs->is_relative_mouse_mode_enabled() )
		{
			// Shift+F1: Show cursor
			if ( inputs->is_key_down( SDL_SCANCODE_LSHIFT ) && inputs->is_key_just_pressed( SDL_SCANCODE_F1 ) )
			{
				inputs->set_relative_mouse_mode( false );

				// TODO: Make a method in InputManager
				const Vec2 window_center = engine.get_window()->get_size() * 0.5f;
				SDL_WarpMouseInWindow( engine.get_window()->get_sdl_window(), window_center.x, window_center.y );
			}

			// Wheel: Scroll to control camera speed
			if ( inputs->mouse_wheel.y != 0.0f )
			{
				editor_camera_speed = math::clamp( editor_camera_speed + editor_camera_speed * inputs->mouse_wheel.y * 0.2f, 0.1f, 1.5f );
			}

			//	Swap X and Y direction so forward is controlled by W/S and right by A/D
			std::swap( dir.x, dir.y );

			// Process input for up and down
			dir.z = inputs->get_keys_as_axis( SDL_SCANCODE_Q, SDL_SCANCODE_E, 0.5f );

			// Rotate in Yaw
			Quaternion yaw_rotation = Quaternion( Vec3::up, math::DEG2RAD * inputs->mouse_delta.x * 30.0f * dt );
			transform->set_rotation( Quaternion::concatenate( transform->rotation, yaw_rotation ) );

			// Rotate in Pitch
			Quaternion pitch_rotation = Quaternion( transform->get_right(), math::DEG2RAD * inputs->mouse_delta.y * 30.0f * dt );
			transform->set_rotation( Quaternion::concatenate( transform->rotation, pitch_rotation ) );

			// Apply location
			const Vec3 world_dir = Vec3::transform( dir, transform->rotation );
			transform->set_location( transform->location + world_dir * ( move_speed * editor_camera_speed * dt ) );
		}
		// LMB: Capture focus
		else if ( inputs->is_mouse_button_just_pressed( MouseButton::Left ) || inputs->is_mouse_button_just_pressed( MouseButton::Right ) )
		{
			inputs->set_relative_mouse_mode( true );
		}
	}
	else
	{
		update_arm_length( dt );

		dir.normalize2d();

		Vec3 pos = transform->location;
		if ( dir == Vec3::zero )
		{
			if ( focus_target.is_valid() )
			{
				pos = focus_target->location;
			}
		}
		else
		{
			// Break focus target reference
			focus_target.reset();

			// Flatten forward (get rid of camera's pitch)
			Vec3 forward = transform->get_forward();
			forward.normalize2d();

			Vec3 right = transform->get_right();

			// Transform direction according to camera's orientation
			dir = dir.x * right
				+ dir.y * forward;

			// Compute final movement vector 
			Vec3 movement = dir * ( move_speed * dt );
			pos = transform->location + movement;
		}

		// Apply new location
		transform->set_location( pos );
	}
}

void CameraController::update_arm_length( float dt )
{
	auto& engine = Engine::instance();
	auto inputs = engine.get_inputs();

	// Control target arm length with mouse wheel
	if ( inputs->mouse_wheel.y )
	{
		target_arm_length = math::clamp(
			target_arm_length + target_arm_length * inputs->mouse_wheel.y * 0.25f,
			1.0f,
			500.0f
		);
	}

	// Smooth arm length towards target arm length
	arm_length = math::lerp( arm_length, target_arm_length, dt * 8.0f );

	// Auto-compute arm length for the first time
	float offset_length = camera_offset.length();
	if ( target_arm_length < 0.0f )
	{
		target_arm_length = offset_length;
	}

	// Update camera offset
	const Vec3 new_offset = camera_offset * ( offset_length / arm_length );
	engine.camera->set_offset( new_offset );
}
