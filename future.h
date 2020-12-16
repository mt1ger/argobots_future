#ifndef _FUTURE_H
#define _FUTURE_H


#include<abt.h>
#include<cstdlib>
#include<iostream>
#include<chrono>
#include<memory> // smart pointer
#include<vector>
#include<iterator>
#include<functional>
#include<type_traits> // is_same

#include "thread.h"


namespace stdx 
{
	/* "launch" related */
	enum class launch  
	{
		async = 1,	
		deferred = 2	
	};
	constexpr launch operator&(launch __x, launch __y)
	{
		return static_cast<launch>(
				static_cast<int>(__x) & static_cast<int>(__y));
	}
	constexpr launch operator|(launch __x, launch __y)
	{
		return static_cast<launch>(
				static_cast<int>(__x) | static_cast<int>(__y));
	}
	constexpr launch operator^(launch __x, launch __y)
	{
		return static_cast<launch>(
				static_cast<int>(__x) ^ static_cast<int>(__y));
	}
	constexpr launch operator~(launch __x)
	{ return static_cast<launch>(~static_cast<int>(__x)); }
	inline launch& operator&=(launch& __x, launch __y)
	{ return __x = __x & __y; }
	inline launch& operator|=(launch& __x, launch __y)
	{ return __x = __x | __y; }
	inline launch& operator^=(launch& __x, launch __y)
	{ return __x = __x ^ __y; }


	enum class future_status 
	{
		ready,
		timeout,
		deferred
	};

	template<class future_T>
	struct shared_state
	{
		future_T ret_value_;
	};

	// template<class promise_T>
	// class promise;

	template<class Ret, class ...Args>
	struct future_wrapper_args
	{
		std::tuple<Args...> tuple_;
		std::function<Ret(Args...)> func_;
		shared_ptr<shared_state<Ret>> ss_ptr_in_fwa;
		// typedef typename result_of<Fn(Args...)>::type my_type_;
		// stdx::future<Ret>* future_ptr_;
	};

	template<class future_T>
	class future
	{
		stdx::thread t1_;
		ABT_eventual future_eventual_;
		int eventual_flag_; // Used to check if the future_eventual_ created
		int deferred_flag_; // Used to indicate the launch policy
		int ready_flag_; // Used to check future_status
		void * args_ptr_; // Used to free async created future_wrapper_args
		shared_ptr<shared_state<future_T>> ss_ptr_; // shared_state pointer
	
		/* To enable get() */
		// template<class Fn, class ...Args>
		// static inline vector<future_wrapper_args<Fn, Args...>*> fwa_vec; 
		// static inline long long fwa_vec_cnt = 0;
		// long long fwa_vec_index;

		int promise_created_flag_;

		template<class Fn, class ...Args>
		friend 
		future<typename result_of<Fn(Args...)>::type>
		async(stdx::launch policy, Fn func_in, Args... args);

		// template<class Fn, class ...Args>
		// friend 
		// void
		// future_wrapper(void * ptr);


		public:
			future (); 
			~future (); 

			future(const future& other) = delete;
			future(future&& other);
			future_T get();

			void wait ();
			bool valid ();

			template <class Rep, class Period>
			stdx::future_status 
			wait_for (const std::chrono::duration<Rep, Period>& time);

			template <class Clock, class Duration>
			stdx::future_status
			wait_until (const chrono::time_point<Clock,Duration>& abs_time);

			// template<class Fn, class ...Args>
			// future_T
			// wrapper(wrapper_args* wa);

			void operator=(future<future_T>&& other);

			template<class Fn, class ...Args>
			inline void
			future_wrapper (void* ptr)  
			{
				stdx::future_wrapper_args<future_T, Args...>*  fwa_ptr;
				fwa_ptr = (stdx::future_wrapper_args<future_T, Args...>*) ptr;
				future_T ret;
				
				ret = std::apply(fwa_ptr->func_, fwa_ptr->tuple_);
				ss_ptr_->ret_value_ = ret;
			}
	};


	template<>
	class future<void>
	{
		stdx::thread t1_;
		ABT_eventual future_eventual_;
		int eventual_flag_; // Used to check if the future_eventual_ created
		int deferred_flag_; // Used to indicate the launch policy
		int ready_flag_; // Used to check future_status
		void * args_ptr_; // Used to free async created future_wrapper_args
	
		int promise_created_flag_;

		template<class Fn, class ...Args>
		friend 
		future<typename result_of<Fn(Args...)>::type>
		async(stdx::launch policy, Fn func_in, Args... args);

		// template<class Fn, class ...Args>
		// friend 
		// void
		// future_wrapper(void * ptr);
		

		public:
			inline future ()
			{
				cout << "in void" << endl;
				eventual_flag_ = 0;
				ready_flag_ = 0;
				deferred_flag_ = 0;
				// fwa_vec_cnt = 0;
				// fwa_vec_index = 0;

				eventual_flag_ = ABT_eventual_create (0, &future_eventual_);
			}
			~future () 
			{
				free(args_ptr_);
				if (eventual_flag_ != 0)
					ABT_eventual_free(&future_eventual_);
			}

			future(const future& other) = delete;
			future(future&& other);
			inline void 
			get()
			{
				if (this->t1_.joinable())
				{
					t1_.join();
				}
			}

			// void wait ();
			// bool valid ();

			// template <class Rep, class Period>
			// stdx::future_status 
			// wait_for (const std::chrono::duration<Rep, Period>& time);
            //
			// template <class Clock, class Duration>
			// stdx::future_status
			// wait_until (const chrono::time_point<Clock,Duration>& abs_time);

			// template<class Fn, class ...Args>
			// future_T
			// wrapper(wrapper_args* wa);

			void 
			operator=(future<void>&& other)
			{
				this->t1_ = std::move(other.t1_);
				this->deferred_flag_ = other.deferred_flag_;
				this->args_ptr_ = nullptr;
				std::swap(this->args_ptr_, other.args_ptr_);
			}


			// Must put future_wrapper_args ahead 
			template<class Fn, class ...Args>
			inline void 
			future_wrapper (void* ptr) 
			{
				stdx::future_wrapper_args<void, Args...>*  fwa_ptr;
				fwa_ptr = (stdx::future_wrapper_args<void, Args...>*) ptr;
				std::apply(fwa_ptr->func_, fwa_ptr->tuple_);	
			}
	};


	template<class promise_T>
	class promise 
	{

		shared_state<promise_T>  ss_;
		ABT_eventual promise_eventual_;

		template<class arg_T>
		friend 
		future<arg_T>
		async(void(*func)(void*), arg_T args);


		friend class stdx::thread;

		public:
			promise(){
				ABT_eventual_create(0, &promise_eventual_);
			}
			~promise(){}

			future<promise_T>
			get_future();

			template<class set_value_T>
			void 
			set_value(set_value_T && arg_in);	

			template<class set_value_T>
			void
			set_value_at_thread_exit (set_value_T && arg_in);
	};


	template<class Ret, class ...Args>
	class packaged_task 
	{
		Ret (*func_) (Args...);
		ABT_eventual pt_eventual_;

		public:
			/* Constructor */
			packaged_task(){};

			template<class Fn>
			packaged_task(Fn && func) noexcept;
			packaged_task(packaged_task &) = delete;
			packaged_task(packaged_task && other) noexcept;
			/* Destructor */
			~packaged_task(){};
	};


	template<class Fn, class ...Args>
	future<typename result_of<Fn(Args...)>::type>
	async (launch policy, Fn func_in, Args ...args);

	// template<class Fn, class ...Args>
	// void
	// future_wrapper(void* ptr);
}

#include"class_future.cpp"
#include"FuncInFuture.cpp"
#include"class_promise.cpp"
#endif


