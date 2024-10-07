#pragma once

#include <suprengine/component.h>

namespace eks
{
	using namespace suprengine;

	template <typename OwnerType>
	class State;
	template <typename OwnerType>
	class StateMachine;

	template <typename OwnerType>
	class StateTask
	{
	public:
		virtual ~StateTask() {}

		virtual void on_begin() {};
		virtual void on_update( float dt ) {};
		virtual void on_end() {};

	public:
		State<OwnerType>* state = nullptr;
	};

	template <typename OwnerType>
	class State
	{
	public:
		virtual ~State()
		{
			for ( auto task : _tasks )
			{
				delete task;
			}
		}

		virtual void on_begin() {};
		virtual void on_update( float dt ) {};
		virtual void on_end() {};

		template <typename TaskType, typename... Args>
		std::enable_if_t<std::is_base_of<StateTask<OwnerType>, TaskType>::value, TaskType*> create_task( Args... args )
		{
			TaskType* task = new TaskType( args... );
			task->state = this;

			_tasks.push_back( task );
			return task;
		}

		void switch_task( int id )
		{
			assert( 0 <= id && id < _tasks.size() );

			//  Prevent switching to the same task
			if ( _current_task_id == id ) return;

			//  End previous task
			_tasks[_current_task_id]->on_end();

			//  Start new task
			_current_task_id = id;
			_tasks[_current_task_id]->on_begin();
		}
		void next_task()
		{
			int next_id = ( _current_task_id + 1 ) % _tasks.size();
			switch_task( next_id );
		}

		StateTask<OwnerType>* get_current_task()
		{
			if ( _tasks.size() == 0 ) return nullptr;
			return _tasks[_current_task_id];
		}

	public:
		StateMachine<OwnerType>* machine = nullptr;

	private:
		int _current_task_id = 0;
		std::vector<StateTask<OwnerType>*> _tasks {};
	};

	template <typename OwnerType>
	class StateMachine : public Component
	{
	public:
		virtual ~StateMachine()
		{
			for ( auto state : _states )
			{
				delete state;
			}
		}

		virtual void update( float dt ) override
		{
			if ( _current_state == nullptr ) return;

			auto current_task = _current_state->get_current_task();
			if ( current_task != nullptr )
			{
				current_task->on_update( dt );
			}
			_current_state->on_update( dt );
		}

		template <typename StateType, typename... Args>
		std::enable_if_t<std::is_base_of<State<OwnerType>, StateType>::value, StateType*> create_state( Args... args )
		{
			StateType* state = new StateType( args... );
			state->machine = this;
			
			_states.push_back( state );
			return state;
		}

		void switch_state( State<OwnerType>* state )
		{
			if ( _current_state != nullptr )
			{
				_current_state->on_end();
				delete _current_state;
			}

			_current_state = state;

			if ( _current_state != nullptr )
			{
				_current_state->on_begin();
			}
		}

	public:
		OwnerType* owner = nullptr;

	private:
		State<OwnerType>* _current_state = nullptr;
		std::vector<State<OwnerType>*> _states {};
	};
}