#pragma once

#include <ekosystem/components/state-machine.h>

#include "pawn.h"

namespace eks
{
	class PawnFindPawnStateTask : public StateTask<Pawn>
	{
	public:
		PawnFindPawnStateTask() {};

		void on_update( float dt ) override
		{
			printf( "PawnFindPawnStateTask update\n" );
			state->next_task();
		}
	};

	class PawnMoveStateTask : public StateTask<Pawn>
	{
	public:
		PawnMoveStateTask() {};

		void on_begin() override
		{
			_timer = 0.0f;
		}
		void on_update( float dt ) override
		{
			printf( "PawnMoveStateTask update\n" );
			
			_timer += dt;
			if ( _timer > 2.0f )
			{
				state->next_task();
			}
		}

	private:
		float _timer = 0.0f;
	};

	class PawnEatStateTask : public StateTask<Pawn>
	{
	public:
		PawnEatStateTask() {};

		void on_update( float dt ) override
		{
			printf( "PawnEatStateTask update\n" );
			state->next_task();
		}
	};

	class PawnChaseState : public State<Pawn>
	{
	public:
		PawnChaseState()
		{
			create_task<PawnFindPawnStateTask>();
			create_task<PawnMoveStateTask>();
			create_task<PawnEatStateTask>();
		}

		void on_update( float dt ) override
		{
			printf( "PawnChaseState update\n" );
		}
	};
}