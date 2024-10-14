#pragma once

#include <suprengine/component.h>

namespace eks
{
	using namespace suprengine;

	template <typename OwnerType>
	class State;
	template <typename OwnerType>
	class StateMachine;

	/*
	 * Enum representing a StateTask's execution result.
	 */
	enum class StateTaskResult
	{
		/*
		 * Result has not been evaluated yet.
		 * The task could be pending or still running.
		 */
		None,
		/*
		 * Task succeed achieving its goal.
		 */
		Succeed,
		/*
		 * Task failed during execution.
		 */
		Failed,
		/*
		 * Task has been canceled by the state machine.
		 * This can happen when switching states.
		 * 
		 * You should avoid using it yourself when finishing tasks.
		 */
		Canceled,
	};

	/*
	 * Templated class defining one of many actions that a state can have.
	 */
	template <typename OwnerType>
	class StateTask
	{
	public:
		virtual ~StateTask() {}

		virtual void on_begin() {};
		virtual void on_update( float dt ) {};
		virtual void on_end() {};

		/*
		 * Returns whenever the task is ready to end earlier.
		 * This is used by the state machine to know if it can switch 
		 * to a more appropriate state.
		 */
		virtual bool can_switch_from() const
		{
			return true;
		}

		/*
		 * Finishes the task with the given result.
		 * It doesn't do anything if the task has already been finished.
		 */
		void finish( StateTaskResult result )
		{
			if ( last_result != StateTaskResult::None ) return;
			last_result = result;
		}

	public:
		State<OwnerType>* state = nullptr;
		StateTaskResult last_result = StateTaskResult::None;
	};

	/*
	 * Templated class defining one of many states that a state machine can have.
	 * 
	 * A state is composed of tasks in a specific and important order, defined as
	 * states are created. The state's tasks are run one after another until one
	 * fail or they all succeed. In any of these two cases, the state start again
	 * at the first task in the order.
	 */
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

		/*
		 * Returns whenever the state can be switched to.
		 * This is used by the state machine to find an appropriate state.
		 */
		virtual bool can_switch_to() const
		{
			return true;
		}
		/*
		 * Returns whenever the state is ready to end earlier.
		 * This is used by the state machine to know if it can switch 
		 * to a more appropriate state.
		 */
		virtual bool can_switch_from() const
		{
			auto task = get_current_task();
			if ( task == nullptr ) return true;
			return task->can_switch_from();
		}

		/*
		 * Creates a task that the state owns and inserts it in its vector of tasks.
		 * Returns the created task.
		 */
		template <typename TaskType, typename... Args>
		std::enable_if_t<std::is_base_of_v<StateTask<OwnerType>, TaskType>, TaskType*> create_task( Args... args )
		{
			TaskType* task = new TaskType( args... );
			task->state = this;

			_tasks.push_back( task );
			return task;
		}

		void switch_task( int id )
		{
			ASSERT( 0 <= id && id < _tasks.size(), "Index 'id' is out-of-range" );

			//  End previous task
			if ( _current_task_id != invalid_id )
			{
				_tasks[_current_task_id]->on_end();
			}

			//  Start new task
			_current_task_id = id;
			_tasks[_current_task_id]->last_result = StateTaskResult::None;
			_tasks[_current_task_id]->on_begin();
		}
		void next_task()
		{
			int next_id = ( _current_task_id + 1 ) % _tasks.size();
			switch_task( next_id );
		}

		StateTask<OwnerType>* get_current_task() const
		{
			if ( _current_task_id == invalid_id ) return nullptr;
			if ( _tasks.size() == 0 ) return nullptr;
			return _tasks[_current_task_id];
		}

		int get_current_task_id() const
		{
			return _current_task_id;
		}

	public:
		static const int invalid_id = -1;

	public:
		StateMachine<OwnerType>* machine = nullptr;

	private:
		int _current_task_id = invalid_id;
		std::vector<StateTask<OwnerType>*> _tasks {};
	};

	/*
	 * Templated component handling all different states for an entity
	 * of a given type. Designed for AI purposes.
	 * 
	 * The machine is composed of states which are, themselves, composed of tasks.
	 * It is limited to run one state and task per update to avoid infinite looping.
	 * 
	 * It will also automatically select the first runnable state, defined by
	 * State::can_switch_from. The order of states' creation is important as well,
	 * as it defines the order of choice.
	 * 
	 * If you want a manual mode, you'll need to tweak a part of the code.
	 * Look for the related note comment.
	 */
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

		virtual void setup() override
		{
			owner = dynamic_cast<OwnerType*>( get_owner().get() );
			ASSERT(
				owner != nullptr,
				"This state machine component is attached to an incorrect owner type!"
			);
		}
		virtual void update( float dt ) override
		{
			auto next_state = _current_state;
			if ( _current_state == nullptr || _current_state->can_switch_from() )
			{
				//	Select the first runnable state
				//	NOTE: This code prevents manual user control over the state
				//		  that should run. An enum indicating the state machine
				//		  mode could help to choose between manual and automatic
				//		  modes, but that's not what I need right now.
				for ( int i = 0; i < _states.size(); i++ )
				{
					auto state = _states[i];
					if ( !state->can_switch_to() ) continue;
		
					next_state = state;
					break;
				}
			}

			//	Switch to the next state
			if ( next_state != _current_state )
			{
				switch_state( next_state );
			}

			if ( _current_state == nullptr ) return;

			//	Initialize the state to its first task
			if ( _current_state->get_current_task_id() == State<OwnerType>::invalid_id )
			{
				_current_state->switch_task( 0 );
			}

			//	Update the state
			_current_state->on_update( dt );

			auto current_task = _current_state->get_current_task();
			if ( current_task == nullptr ) return;

			//	Update the current task only if there is no result yet
			//	NOTE: This prevents running the update method when the result
			//		  has already been set in the begin method.
			if ( current_task->last_result == StateTaskResult::None )
			{
				current_task->on_update( dt );
			}

			//	Listen to task's result and decide which task should be run next
			//	NOTE: This is done once per update to prevent infinite looping on tasks
			//		  that may already finish in the begin method.
			switch ( current_task->last_result )
			{
				case StateTaskResult::Succeed:
					_current_state->next_task();
					break;
				case StateTaskResult::Failed:
				case StateTaskResult::Canceled:
					_current_state->switch_task( 0 );
					break;
			}
		}

		/*
		 * Creates a state that the machine owns and inserts it in its vector of states.
		 * Returns the created state.
		 */
		template <typename StateType, typename... Args>
		std::enable_if_t<std::is_base_of<State<OwnerType>, StateType>::value, StateType*> create_state( Args... args )
		{
			StateType* state = new StateType( args... );
			state->machine = this;
			
			_states.push_back( state );
			return state;
		}

		/*
		 * Switches to a given state.
		 * It handles last task's and state's ends.
		 */
		void switch_state( State<OwnerType>* state )
		{
			if ( _current_state != nullptr )
			{
				//	Cancel current task if no result has already been set
				if ( auto task = _current_state->get_current_task() )
				{
					task->finish( StateTaskResult::Canceled );
				}

				_current_state->on_end();
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