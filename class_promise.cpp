#ifndef _CLASS_PROMISE_CPP
#define _CLASS_PROMISE_CPP

#include"future_test.h"

template<class promise_T>
stdx::future<promise_T>
stdx::promise<promise_T>::get_future() 
{
	future<promise_T> fut;
	fut.ptr = this;
	fut.ss_ptr = &__ss;
	return fut;		
}


template<class promise_T>
template<class set_value_T>
void
stdx::promise<promise_T>::set_value(set_value_T&& arg_in)
{
	__ss.value = arg_in;	
}
#endif
