#ifndef _FUNCINFUTURE_CPP
#define _FUNCINFUTURE_CPP

#include"future.h"

template<class Fn, class ...Args>
stdx::future<typename result_of<Fn(Args...)>::type>
stdx::async (stdx::launch policy, Fn func_in, Args ...args) 
{
	typedef typename std::result_of<decltype(func_in)&(Args...)>::type mytype_;
	future<mytype_> fut;

	future_wrapper_args<mytype_, Args...> * fwa_ptr;
	fwa_ptr = new future_wrapper_args<mytype_, Args...>; 
	fut.args_ptr_ = fwa_ptr;

	fwa_ptr->func_ = func_in;
	fwa_ptr->tuple_ = std::make_tuple(args...); 
	// fwa_ptr->eventual_ = fut.future_eventual_;

	if (policy == stdx::launch::async) 
	{
		fut.t1_ = stdx::thread([&fut](void * ptr){fut.template future_wrapper_async <decltype(fwa_ptr->func_), Args...> (ptr);}, fwa_ptr);
	}
	else if (policy == stdx::launch::deferred) 
	{
		fut.deferred_flag_ = 1;
		fut.t1_ = stdx::thread([&fut](void* ptr)  
								{
									fut.template future_wrapper_deferred <decltype(fwa_ptr->func_), Args...> (ptr);
								}, fwa_ptr);
	}

	return fut;
}

#endif
