#ifndef _CLASS_FUTURE_CPP
#define _CLASS_FUTURE_CPP

#include "future.h"


template<class future_T>
stdx::future<future_T>::future () 
{
	cout << "in future_T" << endl;
	eventual_flag_ = 0;
	ready_flag_ = 0;
	deferred_flag_ = 0;
	no_ret_flag = 0;
	// fwa_vec_cnt = 0;
	// fwa_vec_index = 0;

	eventual_flag_ = ABT_eventual_create (0, &future_eventual_);
}

template<class future_T>
stdx::future<future_T>::future (future<future_T> && other)
{
	std::swap(this->ss_ptr_, other.ss_ptr_);
	std::swap(this->future_eventual_, other.future_eventual_);
	std::swap(this->deferred_flag_, other.deferred_flag_);
}


// template<class future_T>
// stdx::future<future_T>::~future ()
// {
// 	free(args_ptr_);
// 	free(ss_ptr_);
// 	if (eventual_flag_ != 0)
// 		ABT_eventual_free(&future_eventual_);
// }


template<class future_T>
future_T
stdx::future<future_T>::get () 
{
	// For async created && launch is deferred: return the shared_state in future
	// if (deferred_flag_ == 1) 
	// {
	// 	// stdx::future<future_T>::fwa_vec<Fn, .func_(wa_.future_T_dup_);
	// 	fwa_vec<Fn, Args...>
	// 	return wa_.future_T_dup_; 
	// }
	
	// For async created && launch is async: return the shared_state in future
	if (this->t1_.joinable())
	{
		t1_.join();
		shared_state<future_T>* my_ptr = (shared_state<future_T> *) ss_ptr_;
		future_T ret = my_ptr->ret_value_;
		return ret;
	}
	
	// For promise created future: return shared_state in promise
	// return ss_ptr_->ret_value_;
}

template<class future_T>
void
stdx::future<future_T>::wait () 
{
	if (deferred_flag_ != 1) 
		ABT_eventual_wait(future_eventual_, nullptr);
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
	this->deferred_flag_ = other.deferred_flag_;
	this->no_ret_flag = other.no_ret_flag;
	this->ss_ptr_ = other.ss_ptr_;
} 


// template<class future_T>
// template <class Rep, class Period>
// stdx::future_status 
// stdx::future<future_T>::wait_for (const std::chrono::duration<Rep, Period>& dur)
// {
// 	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now ();
// 	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now ();
// 	ABT_eventual_test(wa_.eventual_, nullptr, &this->ready_flag_);  
// 	if (this->ready_flag_ == 1)	
// 		return stdx::future_status::ready;
// 	else if (this->deferred_flag_ == 1) 
// 	{
// 		return stdx::future_status::deferred;
// 	}
// 	else 
// 	{
// 		while (std::chrono::duration_cast<std::chrono::milliseconds>(end - start) < dur)
// 			end = std::chrono::steady_clock::now();
//
// 		ABT_eventual_test(wa_.eventual_, nullptr, &this->ready_flag_);  
// 		if (this->ready_flag_ == 1)
// 			return stdx::future_status::ready;
// 		else
// 			return stdx::future_status::timeout;
// 	}
// }

// template<class future_T>
// template <class Clock, class Duration>
// stdx::future_status
// stdx::future<future_T>::wait_until (const chrono::time_point<Clock,Duration>& abs_time) 
// {
// 	ABT_eventual_test(wa_.eventual_, nullptr, &this->ready_flag_);  
// 	if (this->ready_flag_ == 1)	
// 		return stdx::future_status::ready;
// 	else if (this->deferred_flag_ == 1) 
// 	{
// 		return stdx::future_status::deferred;
// 	}
// 	else 
// 	{
// 		while (std::chrono::steady_clock::now () < abs_time);
//
// 		ABT_eventual_test(wa_.eventual_, nullptr, &this->ready_flag_);  
// 		if (this->ready_flag == 1)
// 			return stdx::future_status::ready;
// 		else
// 			return stdx::future_status::timeout;
// 	}
// }

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

