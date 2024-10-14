#pragma once

#include <ekosystem/components/state-machine.h>

#include "pawn.h"

namespace eks
{
	class PawnEatStateTask : public StateTask<Pawn>
	{
	public:
		PawnEatStateTask( SafePtr<Pawn>* target_key ) 
			: target_key( target_key )
		{};

		void on_begin() override
		{
			SafePtr<Pawn> target = *target_key;
			if ( !target.is_valid() )
			{
				finish( StateTaskResult::Failed );
				return;
			}

			Pawn* owner = state->machine->owner;

			owner->hunger = math::min(
				owner->hunger + target->data->food_amount,
				owner->data->max_hunger
			);
			target->kill();

			printf(
				"'%s' ate '%s'\n",
				*owner->get_name(), *target->get_name()
			);

			finish( StateTaskResult::Succeed );
		}

	public:
		SafePtr<Pawn>* target_key = nullptr;
	};
}