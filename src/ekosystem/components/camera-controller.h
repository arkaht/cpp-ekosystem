#pragma once

#include <suprengine/core/component.h>

#include <suprengine/math/vec3.h>
#include <suprengine/math/quaternion.h>

namespace eks
{
	using namespace suprengine;

	class CameraController : public Component
	{
	public:
		CameraController(
			float move_speed = 100.0f,
			const Vec3& offset = Vec3::zero
		);

		void setup() override;

		void update( float dt ) override;
		void update_arm_length( float dt );

	public:
		float move_speed;

		// NOTE: The name doesn't make much sense with the current implementation, 
		//		 but it's more like a zoom
		float arm_length = 5000.0f;
		float target_arm_length = -1.0f;

		Vec3 camera_offset = Vec3::zero;
		Vec3 start_location = Vec3::zero;
		Quaternion camera_rotation = Quaternion( DegAngles { -45.0f, -135.0f, 0.0f } );

		SafePtr<Transform> focus_target = nullptr;
	};
}