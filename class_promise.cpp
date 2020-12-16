#ifndef _CLASS_PROMISE_CPP
#define _CLASS_PROMISE_CPP

#include"future.h"

template<class promise_T>
stdx::future<promise_T>
stdx::promise<promise_T>::get_future() 
{
	future<promise_T> fut;
	fut.ss_ptr_ = &ss_;
	fut.wa_.eventual_ = promise_eventual_; 
	return fut;
}


template<class promise_T>
template<class set_value_T>
void
stdx::promise<promise_T>::set_value(set_value_T&& arg_in)
{
	ss_.value_ = arg_in;	
}


template<class promise_T>
template<class set_value_T>
void
stdx::promise<promise_T>::set_value_at_thread_exit(set_value_T && arg_in) 
{
	ss_.value_ = arg_in;
	cout << "what should I do " << ss_.value_ << endl;
	ABT_eventual_set(promise_eventual_, nullptr, 0);
}

#endif
