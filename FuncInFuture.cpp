#ifndef _FUNCINFUTURE_CPP
#define _FUNCINFUTURE_CPP

#include"future_test.h"

template<class arg_T>
// template<class Fn, class ...Args>
stdx::future<arg_T>
stdx::async (void(*func)(void*), arg_T args) 
{
	future<arg_T> fut;
	fut.__wa.func = func;
	fut.__wa.future_T_dup = args;
	
	fut.t1 = thread(stdx::wrapper<arg_T>, &(fut.__wa));

	// return std::move(fut);
	return fut;
}

template<class future_T>
// future_T
void
stdx::wrapper (void* wa) 
{
	typedef typename stdx::future<future_T>::wrapper_args*  type0;
	type0 wa_ptr;
	wa_ptr = (type0)wa;
	//Now future_T can only be a struct pointer
	wa_ptr->func(wa_ptr->future_T_dup);
	wa_ptr->ready_flag = 1;
	ABT_eventual_set(wa_ptr->eventual, nullptr, 0);
}

#endif
