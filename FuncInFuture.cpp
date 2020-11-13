#ifndef _FUNCINFUTURE_CPP
#define _FUNCINFUTURE_CPP

#include"future_test.h"

template<class arg_T>
// template<class Fn, class ...Args>
stdx::future<arg_T>
stdx::async (stdx::launch policy, void(*func)(void*), arg_T args) 
{
	future<arg_T> fut;
	if (policy == stdx::launch::async) 
	{
		fut.wa_.func_ = func;
		fut.wa_.future_T_dup_ = args;
		
		fut.t1_ = thread(stdx::wrapper<arg_T>, &(fut.wa_));
	}
	else if (policy == stdx::launch::deferred) 
	{
		fut.wa_.func = func;
		fut.wa_.future_T_dup_ = args;
		fut.deferred_flag_ = 1;
	}

	// return std::move(fut);
	return fut;
}

template<class future_T>
// future_T
void
stdx::wrapper (void* wa) 
{
	/* transform the input void pointer to wrapper_args pointer */
	typedef typename stdx::future<future_T>::wrapper_args*  type0;
	type0 wa_ptr;
	wa_ptr = (type0)wa;
	
	//Now future_T can only be a struct pointer
	wa_ptr->func_(wa_ptr->future_T_dup_);
	ABT_eventual_set(wa_ptr->eventual_, nullptr, 0);
}

#endif
