#pragma once

#include <ekosystem/components/state-machine.h>

#include <ekosystem/entities/pawn.h>

namespace eks
{
	class PawnMateStateTask : public StateTask<Pawn>
	{
	public:
		PawnMateStateTask( SafePtr<Pawn>* partner_key ) 
			: partner_key( partner_key )
		{};

		void on_begin() override
		{
			SafePtr<Pawn> partner = *partner_key;
			if ( !partner.is_valid() )
			{
				finish( StateTaskResult::Failed );
				return;
			}

			Pawn* owner = state->machine->owner;
			owner->reproduce( partner );

			finish( StateTaskResult::Succeed );
		}

	public:
		SafePtr<Pawn>* partner_key = nullptr;
	};
}