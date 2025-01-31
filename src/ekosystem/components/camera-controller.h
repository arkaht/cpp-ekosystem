#pragma once

#include <suprengine/core/component.h>

namespace eks
{
	using namespace suprengine;

	class CameraController : public Component
	{
	public:
		CameraController(
			float move_speed = 20.0f,
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

		SafePtr<Transform> focus_target;
	};
}