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
	// stdx::future<mytype_>::fwa_vec_cnt++;
	// fut.fwa_vec_index = stdx::future<mytype_>::fwa_vet_cnt - 1;
	// stdx::future<mytype_>::template fwa_vec<Fn, Args...>[fut.fwa_vec_index] = new future_wrapper_args<Fn, Args...>; 
	// stdx::future_wrapper_args<Fn, Args...>* fwa_ptr = stdx::future<mytype_>::template fwa_vec<Fn, Args...>[fut.fwa_vec_index];

	fwa_ptr = new future_wrapper_args<mytype_, Args...>; 
	fut.args_ptr_ = fwa_ptr;

	fwa_ptr->func_ = func_in;
	fwa_ptr->tuple_ = std::make_tuple(args...); 
	fwa_ptr->future_ptr_ = &fut;

	// mytype_  ret;
	// ret = apply(fwa_ptr->func_, fwa_ptr->tuple_);
	apply(fwa_ptr->func_, fwa_ptr->tuple_);
	// cout << "in async ret is " << ret  << endl;
	
	// if (policy == stdx::launch::async) 
	// {
	// 	fut.t1_ = thread(future_wrapper<decltype(fwa_ptr->func_), Args...>, fwa_ptr);
	// }
	// else if (policy == stdx::launch::deferred) 
	// {
	// 	fut.deferred_flag_ = 1;
	// }

	return fut;
}


template<class Fn, class ...Args>
void
stdx::future_wrapper (void* ptr) 
{

	/* transform the input void pointer to wrapper_args pointer */
	cout << "AAAA" << endl;
	typedef typename result_of<Fn(Args...)>::type my_type_;
	stdx::future_wrapper_args<my_type_, Args...>*  fwa_ptr;
	fwa_ptr = (future_wrapper_args<my_type_, Args...>*) ptr;
	cout << "BBBB" << endl;

	printf("is_same %d\n", is_same<my_type_, void>::value);
	// if((is_same<my_type_, void>::value) != 1)	
	// {
	// 	cout << "in future_wrapper my_type_ is int? " << is_same<my_type_, int>::value << endl;
	// 	my_type_ ret;
	// 	shared_state<my_type_>* my_ptr;
	// 	my_ptr = new shared_state<my_type_>;
	// 	ret = std::apply(fwa_ptr->func_, fwa_ptr->tuple_);
	// 	my_ptr->ret_value_ = ret;
	// 	fwa_ptr->future_ptr_->ss_ptr_ = my_ptr;
	// }
	// else
	// {
	// 	std::apply(fwa_ptr->func_, fwa_ptr->tuple_);	
	// 	fwa_ptr->future_ptr_->no_ret_flag = 1;
	// }
	// ABT_eventual_set(fwa_ptr->future_ptr_->future_eventual_, nullptr, 0);
}
#endif
