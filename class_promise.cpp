#ifndef _CLASS_PROMISE_CPP
#define _CLASS_PROMISE_CPP

#include"future.h"

template<class promise_T>
stdx::promise<promise_T>::promise (promise<promise_T> && other) noexcept 
{
	std::swap(this->ss_ptr_, other.ss_ptr_);
	std::swap(this->eventual_ptr_, other.eventual_ptr_);
}


template<class promise_T>
stdx::future<promise_T>
stdx::promise<promise_T>::get_future() 
{
	future<promise_T> fut;
	fut.ss_ptr_ = make_shared<shared_state<promise_T>>();
	ss_ptr_ = fut.ss_ptr_;
	eventual_ptr_= fut.eventual_ptr_;
	fut.promise_created_flag_ = 1;
	return fut;
}

template<class promise_T>
void
stdx::promise<promise_T>::set_value(promise_T && arg_in)
{
	ss_ptr_->ret_value_ = arg_in;	
}

template<class promise_T>
void
stdx::promise<promise_T>::set_value(const promise_T & arg_in)
{
	ss_ptr_->ret_value_ = arg_in;	
}


template<class promise_T>
void
stdx::promise<promise_T>::set_value_at_thread_exit(promise_T && arg_in) 
{
	ss_ptr_->ret_value_ = arg_in;
	ABT_eventual_set(*eventual_ptr_, nullptr, 0);
}

template<class promise_T>
void
stdx::promise<promise_T>::set_value_at_thread_exit(const promise_T & arg_in) 
{
	ss_ptr_->ret_value_ = arg_in;
	ABT_eventual_set(*eventual_ptr_, nullptr, 0);
}


template<class promise_T>
void
stdx::promise<promise_T>::operator=(promise<promise_T> && other) 
{
	this->ss_ptr_ = other.ss_ptr_;
	this->eventual_ptr_ = other.eventual_ptr_;
}

template<class promise_T>
void
stdx::promise<promise_T>::swap(promise<promise_T> && other) 
{
	std::swap(this->ss_ptr_, other.ss_ptr_);
	std::swap(this->eventual_ptr_, other.eventual_ptr_);
}
#endif
