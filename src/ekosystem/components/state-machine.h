#pragma once

#include <suprengine/component.h>

namespace eks
{
	using namespace suprengine;

	class StateMachine;

	class State
	{
	public:
		virtual void on_state_begin() = 0;
		virtual void on_state_update( float dt ) = 0;
		virtual void on_state_end() = 0;

	public:
		StateMachine* state_machine = nullptr;
	};

	class StateMachine : public Component
	{
	public:
		StateMachine() {}

		template<typename T, typename... Args>
		T* create_state( Args... args )
		{
			T* state = new T( args... );
			state->state_machine = this;
			set_state( state );
			return state;
		}
		void set_state( State* state );

	private:
		State* _current_state = nullptr;
	};
}