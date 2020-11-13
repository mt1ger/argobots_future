#ifndef _CLASS_FUTURE_CPP
#define _CLASS_FUTURE_CPP


#include "future_test.h"

template<class future_T>
stdx::future<future_T>::future () 
{
	eventual_flag_ = 0;
	ready_flag_ = 0;
	deferred_flag_ = 0;

	eventual_flag_ = ABT_eventual_create (0, &wa_.eventual_);
}

template<class future_T>
stdx::future<future_T>::future (future<future_T> && other)
{
	std::swap(this->ss_ptr_, other.ss_ptr_);
	std::swap(this->wa_.eventual_, other.wa_.eventual_);
}


template<class future_T>
stdx::future<future_T>::~future ()
{
	if (eventual_flag_ != 0)
		ABT_eventual_free(&wa_.eventual_);
}


template<class future_T>
future_T
stdx::future<future_T>::get () 
{
	// Only deal with function (void *) now 
	// For async created && launch is deferred: return the shared_state in future
	if (deferred_flag_ == 1) 
	{
		wa_.func(wa_.future_T_dup_);
		return wa_.future_T_dup_; 
	}
	
	// For async created && launch is async: return the shared_state in future
	if (this->t1_.joinable())
	{
		t1_.join();
		return wa_.future_T_dup_;
	}
	
	// For promise created future: return shared_state in promise
	return ss_ptr_->value_;
}


template<class future_T>
void
stdx::future<future_T>::wait () 
{
	ABT_eventual_wait(wa_.eventual_, nullptr);
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
	std::swap(this->ss_ptr_, other.ss_ptr_);
	this->t1_ = std::move(other.t1_);
	this->wa_ = other.wa_;
} 


template<class future_T>
template <class Rep, class Period>
stdx::future_status 
stdx::future<future_T>::wait_for (const std::chrono::duration<Rep, Period>& dur)
{
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now ();
	ABT_eventual_test(wa_.eventual_, nullptr, &this->ready_flag_);  
	if (this->ready_flag_ == 1)	
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

		ABT_eventual_test(wa_.eventual_, nullptr, &this->ready_flag_);  
		if (this->ready_flag_ == 1)
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
	ABT_eventual_test(wa_.eventual_, nullptr, &this->ready_flag_);  
	if (this->ready_flag_ == 1)	
		return stdx::future_status::ready;
	
	/* this line is for deferred */
	// else if () 
	// {
	//
	// }
	else 
	{
		while (std::chrono::steady_clock::now () < abs_time);

		ABT_eventual_test(wa_.eventual_, nullptr, &this->ready_flag_);  
		if (this->ready_flag == 1)
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

