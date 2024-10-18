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
			Pawn* owner = state->machine->owner;
			owner->reproduce( partner );

			finish( StateTaskResult::Succeed );
		}

		std::string get_name() const override
		{
			return "PawnMateStateTask";
		}

	public:
		SafePtr<Pawn>* partner_key = nullptr;
	};
}