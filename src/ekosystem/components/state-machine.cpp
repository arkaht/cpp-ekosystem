#include "state-machine.h"

using namespace eks;

void StateMachine::set_state( State* state )
{
	if ( _current_state != nullptr )
	{
		_current_state->on_state_end();
		delete _current_state;
	}

	_current_state = state;

	if ( _current_state != nullptr )
	{
		_current_state->on_state_begin();
	}
}
