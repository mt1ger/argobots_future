#ifndef _CLASS_PACKAGED_TASK_CPP
#define _CLASS_PACKAGED_TASK_CPP

#include"future.h"

template<class Ret, class ...Args>
template<class Fn>
stdx::packaged_task<Ret(Args...)>::packaged_task (Fn && func_in) noexcept 
{
	func_ = func_in;	
}

template<class Ret, class ...Args>
stdx::packaged_task<Ret(Args...)>::packaged_task (stdx::packaged_task<Ret(Args...)> && other) noexcept 
{
	std::swap(this->func_, other.func_);
	std::swap(this->args_, other.args_);
	std::swap(this->ss_ptr_, other.ss_ptr_);
	std::swap(this->eventual_ptr_, other.eventual_ptr_);
}

template<class Ret, class ...Args>
stdx::future<Ret>
stdx::packaged_task<Ret(Args...)>::get_future() 
{
	future<Ret> fut;
	fut.ss_ptr_ = make_shared<shared_state<Ret>>();
	ss_ptr_ = fut.ss_ptr_;
	eventual_ptr_= fut.eventual_ptr_;
	fut.pt_created_flag_ = 1;
	return fut;
}

template<class Ret, class ...Args>
void 
stdx::packaged_task<Ret(Args...)>::make_ready_at_thread_exit (Args ...args) 
{
	args_ = std::make_tuple(args...);
	ss_ptr_->ret_value_ = std::apply (func_, args_);
	ABT_eventual_set(*eventual_ptr_, nullptr, 0);
}

template<class Ret, class ...Args>
stdx::packaged_task<Ret(Args...)>& 
stdx::packaged_task<Ret(Args...)>::operator= (packaged_task<Ret(Args...)> && other) noexcept
{
	this->func_ = other.func_;
	this->args_ = other.args_;
	this->ss_ptr_ = other.ss_ptr_;
	this->eventual_ptr_ = other.eventual_ptr_;
	return *this;
}

template<class Ret, class ...Args>
void 
stdx::packaged_task<Ret(Args...)>::operator()(Args... args) 
{
	args_ = std::make_tuple(args...);
	ss_ptr_->ret_value_ = std::apply (func_, args_);
}

template<class Ret, class ...Args>
void 
stdx::packaged_task<Ret(Args...)>::reset() 
{
	ss_ptr_ = nullptr;
	eventual_ptr_ = nullptr;
}

template<class Ret, class ...Args>
void 
stdx::packaged_task<Ret(Args...)>::swap (packaged_task& other) noexcept 
{
	std::swap(this->func_, other.func_);
	std::swap(this->args_, other.args_);
	std::swap(this->ss_ptr_, other.ss_ptr_);
	std::swap(this->eventual_ptr_, other.eventual_ptr_);
}





/****************** BELOW IS FOR PACKAGED_TASK<VOID(ARGS...)> *************************/

template<class ...Args>
template<class Fn>
stdx::packaged_task<void(Args...)>::packaged_task (Fn && func_in) noexcept 
{
	func_ = func_in;	
}

template<class ...Args>
stdx::packaged_task<void(Args...)>::packaged_task (stdx::packaged_task<void(Args...)> && other) noexcept 
{
	std::swap(this->func_, other.func_);
	std::swap(this->args_, other.args_);
	std::swap(this->eventual_ptr_, other.eventual_ptr_);
}

template<class ...Args>
stdx::future<void>
stdx::packaged_task<void(Args...)>::get_future() 
{
	future<void> fut;
	eventual_ptr_= fut.eventual_ptr_;
	fut.pt_created_flag_ = 1;
	return fut;
}

template<class ...Args>
void 
stdx::packaged_task<void(Args...)>::make_ready_at_thread_exit (Args ...args) 
{
	args_ = std::make_tuple(args...);
	std::apply (func_, args_);
	ABT_eventual_set(*eventual_ptr_, nullptr, 0);
}

template<class ...Args>
stdx::packaged_task<void(Args...)>& 
stdx::packaged_task<void(Args...)>::operator= (packaged_task<void(Args...)> && other) noexcept
{
	this->func_ = other.func_;
	this->args_ = other.args_;
	this->eventual_ptr_ = other.eventual_ptr_;
	return *this;
}

template<class ...Args>
void 
stdx::packaged_task<void(Args...)>::operator()(Args... args) 
{
	args_ = std::make_tuple(args...);
	std::apply (func_, args_);
}

template<class ...Args>
void 
stdx::packaged_task<void(Args...)>::reset() 
{
	eventual_ptr_ = nullptr;
}

template<class ...Args>
void 
stdx::packaged_task<void(Args...)>::swap (packaged_task& other) noexcept 
{
	std::swap(this->func_, other.func_);
	std::swap(this->args_, other.args_);
	std::swap(this->eventual_ptr_, other.eventual_ptr_);
}
#endif

