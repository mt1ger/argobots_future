#ifndef _CLASS_FUTURE_CPP
#define _CLASS_FUTURE_CPP


#include "future_test.h"

template<class future_T>
stdx::future<future_T>::future () 
{
	eventual_flag = 0;
	ready_flag = 0;
	deferred_flag = 0;

	eventual_flag = ABT_eventual_create (0, &__wa.eventual);
}

template<class future_T>
stdx::future<future_T>::future (future<future_T> && other)
{
	std::swap(this->ss_ptr, other.ss_ptr);
	std::swap(this->__wa.eventual, other.__wa.eventual);
}


template<class future_T>
stdx::future<future_T>::~future ()
{
	if (eventual_flag != 0)
		ABT_eventual_free(&__wa.eventual);
}


template<class future_T>
future_T
stdx::future<future_T>::get () 
{
	if (this->t1.joinable())
	{
		t1.join();
		return __wa.future_T_dup;
	}
	return ss_ptr->value;
}


template<class future_T>
void
stdx::future<future_T>::wait () 
{
	ABT_eventual_wait(__wa.eventual, nullptr);
}


// template<class future_T>
// bool
// stdx::future<future_T>::valid()
// {
// 	// if (t1.joinable())
// 	// 	return false;
// 	// else
// 	// 	return true;
// }


template<class future_T>
void 
stdx::future<future_T>::operator=(future<future_T>&& other)
{
	std::swap(this->ss_ptr, other.ss_ptr);
	// std::swap(this->t1, other.t1);
	this->t1 = std::move(other.t1);
	// std::swap(this->__wa, other.__wa);
	this->__wa = other.__wa;
	// std::swap(this->__wa.eventual, other.__wa.eventual);
} 


template<class future_T>
template <class Rep, class Period>
stdx::future_status 
stdx::future<future_T>::wait_for (const std::chrono::duration<Rep, Period>& dur)
{
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now ();
	if (ready_flag == 1)	
		return stdx::future_status::ready;
	
	/* this line is for deferred */
	// else if () 
	// {
	//
	// }
	else 
	{
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now ();
		while (std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(end - start) < time)
			end = std::chrono::steady_clock::now();
		if (ready_flag == 1)
			return stdx::future_status::ready;
		else
			return stdx::future_status::timeout;
	}
}

template<class future_T>
template <class Clock, class Duration>
stdx::future_status
stdx::future<future_T>::wait_until (const chrono::time_point<Clock,Duration>& abs_time) 
{
	if (ready_flag == 1)	
		return stdx::future_status::ready;
	
	/* this line is for deferred */
	// else if () 
	// {
	//
	// }
	else 
	{
		while (std::chrono::steady_clock::now () < abs_time);
		if (ready_flag == 1)
			return stdx::future_status::ready;
		else
			return stdx::future_status::timeout;
	}
}

// template<class future_T>
// template<class Fn, class ...Args>
// future_T
// stdx::future<future_T>::wrapper (Fn func, Args ...args) 
// template<class future_T>
// future_T
// stdx::future<future_T>::wrapper (wrapper_args* wa) 
// {
// 	//Now future_T can only be a struct pointer
// 	wa->func(wa->future_T_dup);
// 	ready_flag = 1;
// 	ABT_eventual_set(eventual, nullptr, 0);
// 	return wa->future_T_dup; 
// }
#endif

