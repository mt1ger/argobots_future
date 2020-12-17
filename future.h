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

	template<class Ret, class ...Args>
	struct future_wrapper_args
	{
		std::tuple<Args...> tuple_;
		std::function<Ret(Args...)> func_;
		// ABT_eventual eventual_;
	};

	template<class future_T>
	class future
	{
		stdx::thread t1_;
		int eventual_flag_; // Used to check if the future_eventual_ created
		int deferred_flag_; // Used to indicate the launch policy
		int ready_flag_; // Used to check future_status
		void * args_ptr_; // Used to free async created future_wrapper_args
		std::shared_ptr<shared_state<future_T>> ss_ptr_; // shared_state pointer
		std::shared_ptr<ABT_eventual> eventual_ptr_;
		bool valid_flag_;
		int promise_created_flag_;

		template<class Fn, class ...Args>
		friend 
		future<typename result_of<Fn(Args...)>::type>
		async(stdx::launch policy, Fn func_in, Args... args);

		template<class promise_T>
		friend
		class promise;

		public:
			future (); 
			~future (); 
			future(const future& other) = delete;
			future(future&& other);

			future_T get();

			bool valid ();

			void wait ();

			template <class Rep, class Period>
			stdx::future_status 
			wait_for (const std::chrono::duration<Rep, Period>& time);

			template <class Clock, class Duration>
			stdx::future_status
			wait_until (const chrono::time_point<Clock,Duration>& abs_time);

			void operator=(future<future_T>&& other);

			template<class Fn, class ...Args>
			inline void
			future_wrapper_async (void* ptr)  
			{
				stdx::future_wrapper_args<future_T, Args...>*  fwa_ptr;
				fwa_ptr = (stdx::future_wrapper_args<future_T, Args...>*) ptr;
				future_T ret;
				
				ret = std::apply(fwa_ptr->func_, fwa_ptr->tuple_);
				ss_ptr_->ret_value_ = ret;
				// ABT_eventual_set(fwa_ptr->eventual_, nullptr, 0);
				ABT_eventual_set(*eventual_ptr_, nullptr, 0);
			}

			template<class Fn, class ...Args>
			inline void
			future_wrapper_deferred (void* ptr)  
			{
				stdx::future_wrapper_args<future_T, Args...>*  fwa_ptr;
				fwa_ptr = (stdx::future_wrapper_args<future_T, Args...>*) ptr;
				ABT_eventual_wait(*eventual_ptr_, nullptr);
				future_T ret;
				
				ret = std::apply(fwa_ptr->func_, fwa_ptr->tuple_);
				ss_ptr_->ret_value_ = ret;
			}
	};


	template<>
	class future<void>
	{
		stdx::thread t1_;
		int eventual_flag_; // Used to check if the future_eventual_ created
		int deferred_flag_; // Used to indicate the launch policy
		int ready_flag_; // Used to check future_status
		void * args_ptr_; // Used to free async created future_wrapper_args
		bool valid_flag_; 
		std::shared_ptr<ABT_eventual> eventual_ptr_;
		int promise_created_flag_;

		template<class Fn, class ...Args>
		friend 
		future<typename result_of<Fn(Args...)>::type>
		async(stdx::launch policy, Fn func_in, Args... args);

		template<class promise_T>
		friend
		class promise;

		public:
			inline future ()
			{
				args_ptr_ = nullptr;
				eventual_flag_ = 0;
				ready_flag_ = 0;
				deferred_flag_ = 0;
				valid_flag_ = 0;

				eventual_ptr_ = std::make_shared<ABT_eventual>();
				eventual_flag_ = ABT_eventual_create (0, &(*eventual_ptr_));
			}
			inline ~future () 
			{
				free(args_ptr_);
				if (eventual_flag_ != 0)
					ABT_eventual_free(&(*eventual_ptr_));
			}

			future(const future& other) = delete;

			future (future<void> && other)
			{
				this->args_ptr_ = nullptr;
				std::swap(this->args_ptr_, other.args_ptr_);
				std::swap(this->eventual_ptr_, other.eventual_ptr_);
				std::swap(this->deferred_flag_, other.deferred_flag_);
				std::swap(this->valid_flag_, other.valid_flag_);
				std::swap(this->ready_flag_, other.ready_flag_);
			}

			inline void 
			get()
			{
				/* For async created && launch is DEFERRED: return the shared_state in future */
				if (deferred_flag_ == 1) 
				{
					ABT_eventual_set(*eventual_ptr_, nullptr, 0);
					if (this->t1_.joinable())
						t1_.join ();
				}
				/* For async created && launch is ASYNC: return the shared_state in future */
				else
				{
					if (this->t1_.joinable())
						t1_.join();
				}
			}

			inline void wait () 
			{
				if (deferred_flag_ != 1)
					ABT_eventual_wait(*eventual_ptr_, nullptr);
				else	
					cout << "future is in deferred status" << endl;
			}
			
			inline bool valid () 
			{
				return valid_flag_;
			}

			template <class Rep, class Period>
			stdx::future_status 
			wait_for (const std::chrono::duration<Rep, Period>& dur)  
			{
				std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now ();
				std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now ();
				ABT_eventual_test(*eventual_ptr_, nullptr, &this->ready_flag_);  
				if (this->ready_flag_ == 1)	
					return stdx::future_status::ready;
				else if (this->deferred_flag_ == 1) 
				{
					return stdx::future_status::deferred;
				}
				else 
				{
					while (std::chrono::duration_cast<std::chrono::milliseconds>(end - start) < dur)
						end = std::chrono::steady_clock::now();

					ABT_eventual_test(*eventual_ptr_, nullptr, &this->ready_flag_);  
					if (this->ready_flag_ == 1)
						return stdx::future_status::ready;
					else
						return stdx::future_status::timeout;
				}

			}
            
			template <class Clock, class Duration>
			stdx::future_status
			wait_until (const chrono::time_point<Clock,Duration>& abs_time) 
			{
				ABT_eventual_test(*eventual_ptr_, nullptr, &this->ready_flag_);  
				if (this->ready_flag_ == 1)	
					return stdx::future_status::ready;
				else if (this->deferred_flag_ == 1) 
				{
					return stdx::future_status::deferred;
				}
				else 
				{
					while (std::chrono::steady_clock::now () < abs_time);

					ABT_eventual_test(*eventual_ptr_, nullptr, &this->ready_flag_);  
					if (this->ready_flag_ == 1)
						return stdx::future_status::ready;
					else
						return stdx::future_status::timeout;
				}
			}


			void 
			operator=(future<void>&& other)
			{
				this->t1_ = std::move(other.t1_);
				this->deferred_flag_ = other.deferred_flag_;
				this->eventual_ptr_ = other.eventual_ptr_;
				this->valid_flag_= other.valid_flag_;
				this->ready_flag_= other.ready_flag_;
				this->args_ptr_ = nullptr;
				std::swap(this->args_ptr_, other.args_ptr_);
			}


			// Must put future_wrapper_args ahead 
			template<class Fn, class ...Args>
			inline void 
			future_wrapper_async (void* ptr) 
			{
				stdx::future_wrapper_args<void, Args...>*  fwa_ptr;
				fwa_ptr = (stdx::future_wrapper_args<void, Args...>*) ptr;
				std::apply(fwa_ptr->func_, fwa_ptr->tuple_);	
				// ABT_eventual_set(fwa_ptr->eventual_, nullptr, 0);
				ABT_eventual_set(*eventual_ptr_, nullptr, 0);
			}

			template<class Fn, class ...Args>
			inline void
			future_wrapper_deferred (void* ptr)  
			{
				stdx::future_wrapper_args<void, Args...>*  fwa_ptr;
				fwa_ptr = (stdx::future_wrapper_args<void, Args...>*) ptr;
				// ABT_eventual_wait(fwa_ptr->eventual_, nullptr);
				ABT_eventual_wait(*eventual_ptr_, nullptr);
				std::apply(fwa_ptr->func_, fwa_ptr->tuple_);
			}
	};


	template<class promise_T>
	class promise 
	{

		std::shared_ptr<shared_state<promise_T>> ss_ptr_;
		std::shared_ptr<ABT_eventual> eventual_ptr_;
		// ABT_eventual promise_eventual_;

		public:
			promise(){}
			~promise(){}

			future<promise_T>
			get_future();

			void 
			set_value(promise_T && arg_in);	
			void 
			set_value(const promise_T & arg_in);	

			void
			set_value_at_thread_exit (promise_T && arg_in);
			void
			set_value_at_thread_exit (const promise_T & arg_in);
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


