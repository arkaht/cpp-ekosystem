#pragma once

#include <suprengine/component.h>

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
	
	public:
		float move_speed;
		Vec3 offset;
		SafePtr<Transform> focus_target;
	};
}