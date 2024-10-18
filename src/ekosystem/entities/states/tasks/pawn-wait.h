#pragma once

#include <ekosystem/components/state-machine.h>

#include <ekosystem/entities/pawn.h>

namespace eks
{
	class PawnWaitStateTask : public StateTask<Pawn>
	{
	public:
		PawnWaitStateTask( float wait_time, float random_deviation = 0.0f )
			: wait_time( wait_time ), random_deviation( random_deviation )
		{}

		void on_begin() override
		{
			_current_time = 0.0f;

			_max_time = wait_time;

			//	Apply random deviation to time
			if ( random_deviation != 0.0f )
			{
				_max_time = math::max(
					0.0f,
					_max_time + random::generate( -random_deviation, random_deviation )
				);
			}
		}
		void on_update( float dt ) override
		{
			if ( ( _current_time += dt ) < _max_time ) return;

			finish( StateTaskResult::Succeed );
		}

		std::string get_name() const override
		{
			return "PawnWaitStateTask";
		}

	public:
		float wait_time;
		float random_deviation;

	private:
		float _current_time = 0.0f;
		float _max_time = 0.0f;
	};
}