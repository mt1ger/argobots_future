#ifndef _FUTURE_H
#define _FUTURE_H



#include <mutex>
#include <thread>
#include <condition_variable>
#include <system_error>
#include <atomic>
#include<future>
// #include <bits/atomic_futex.h>
// #include <bits/functexcept.h>
// #include <bits/invoke.h>
// #include <bits/unique_ptr.h>
// #include <bits/shared_ptr.h>
// #include <bits/std_function.h>
// #include <bits/uses_allocator.h>
// #include <bits/allocated_ptr.h>
// #include <ext/aligned_buffer.h>

#include<abt.h>
#include<cstdlib>
#include<iostream>

#include "singleton.h"
#include "thread.h"


namespace stdx1 
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


	struct __future_base
	{
		/// Base class for results.
		struct _Result_base
		{
			exception_ptr                _M_error;
			_Result_base(const _Result_base&) = delete;
			_Result_base& operator=(const _Result_base&) = delete;
			// _M_destroy() allows derived classes to control deallocation
			virtual void _M_destroy() = 0;
			struct _Deleter
			{
				void operator()(_Result_base* __fr) const { __fr->_M_destroy(); }
			};
			protected:
			_Result_base();
			virtual ~_Result_base();
		};
		
		/// A unique_ptr for result objects.
		template<typename _Res>
			using _Ptr = unique_ptr<_Res, _Result_base::_Deleter>;

		/// A result object that has storage for an object of type _Res.
		template<typename _Res>
		struct _Result : _Result_base
		{
			private:
				__gnu_cxx::__aligned_buffer<_Res>        _M_storage;
				bool                                         _M_initialized;
			public:
				typedef _Res result_type;
				_Result() noexcept : _M_initialized() { }
				~_Result()
				{
					if (_M_initialized)
						_M_value().~_Res();
				}
				// Return lvalue, future will add const or rvalue-reference
				_Res&
					_M_value() noexcept { return *_M_storage._M_ptr(); }
				void
					_M_set(const _Res& __res)
					{
						::new (_M_storage._M_addr()) _Res(__res);
						_M_initialized = true;
					}
				void
					_M_set(_Res&& __res)
					{
						::new (_M_storage._M_addr()) _Res(std::move(__res));
						_M_initialized = true;
					}
			private:
				void _M_destroy() { delete this; }
		};
		// /// A result object that uses an allocator.
		// template<typename _Res, typename _Alloc>
		// 	struct _Result_alloc final : _Result<_Res>, _Alloc
		// {
		// 	using __allocator_type = __alloc_rebind<_Alloc, _Result_alloc>;
		// 	explicit
		// 		_Result_alloc(const _Alloc& __a) : _Result<_Res>(), _Alloc(__a)
		// 	{ }
		// 	private:
		// 	void _M_destroy()
		// 	{
		// 		__allocator_type __a(*this);
		// 		__allocated_ptr<__allocator_type> __guard_ptr{ __a, this };
		// 		this->~_Result_alloc();
		// 	}
		// };
		// Create a result object that uses an allocator.
		// template<typename _Res, typename _Allocator>
		// 	static _Ptr<_Result_alloc<_Res, _Allocator>>
		// 	_S_allocate_result(const _Allocator& __a)
		// 	{
		// 		using __result_type = _Result_alloc<_Res, _Allocator>;
		// 		typename __result_type::__allocator_type __a2(__a);
		// 		auto __guard = std::__allocate_guarded(__a2);
		// 		__result_type* __p = ::new((void*)__guard.get()) __result_type{__a};
		// 		__guard = nullptr;
		// 		return _Ptr<__result_type>(__p);
		// 	}
		// Keep it simple for std::allocator.
		// template<typename _Res, typename _Tp>
		// 	static _Ptr<_Result<_Res>>
		// 	_S_allocate_result(const std::allocator<_Tp>& __a)
		// 	{
		// 		return _Ptr<_Result<_Res>>(new _Result<_Res>);
		// 	}
		// Base class for various types of shared state created by an
		// asynchronous provider (such as a std::promise) and shared with one
		// or more associated futures.
		class _State_baseV2
		{
			typedef _Ptr<_Result_base> _Ptr_type;
			enum _Status : unsigned {
				__not_ready,
				__ready
			};
			_Ptr_type                        _M_result;
			__atomic_futex_unsigned<>        _M_status;
			atomic_flag                 _M_retrieved = ATOMIC_FLAG_INIT;
			once_flag                        _M_once;
			public:
			_State_baseV2() noexcept : _M_result(), _M_status(_Status::__not_ready)
			{ }
			_State_baseV2(const _State_baseV2&) = delete;
			_State_baseV2& operator=(const _State_baseV2&) = delete;
			virtual ~_State_baseV2() = default;
			_Result_base&
				wait()
				{
					// Run any deferred function or join any asynchronous thread:
					_M_complete_async();
					// Acquire MO makes sure this synchronizes with the thread that made
					// the future ready.
					_M_status._M_load_when_equal(_Status::__ready, memory_order_acquire);
					return *_M_result;
				}
			template<typename _Rep, typename _Period>
				future_status
				wait_for(const chrono::duration<_Rep, _Period>& __rel)
				{
					// First, check if the future has been made ready.  Use acquire MO
					// to synchronize with the thread that made it ready.
					if (_M_status._M_load(memory_order_acquire) == _Status::__ready)
						return future_status::ready;
					if (_M_is_deferred_future())
						return future_status::deferred;
					if (_M_status._M_load_when_equal_for(_Status::__ready,
								memory_order_acquire, __rel))
					{
						// _GLIBCXX_RESOLVE_LIB_DEFECTS
						// 2100.  timed waiting functions must also join
						// This call is a no-op by default except on an async future,
						// in which case the async thread is joined.  It's also not a
						// no-op for a deferred future, but such a future will never
						// reach this point because it returns future_status::deferred
						// instead of waiting for the future to become ready (see
						// above).  Async futures synchronize in this call, so we need
						// no further synchronization here.
						_M_complete_async();
						return future_status::ready;
					}
					return future_status::timeout;
				}
			template<typename _Clock, typename _Duration>
				future_status
				wait_until(const chrono::time_point<_Clock, _Duration>& __abs)
				{
					// First, check if the future has been made ready.  Use acquire MO
					// to synchronize with the thread that made it ready.
					if (_M_status._M_load(memory_order_acquire) == _Status::__ready)
						return future_status::ready;
					if (_M_is_deferred_future())
						return future_status::deferred;
					if (_M_status._M_load_when_equal_until(_Status::__ready,
								memory_order_acquire, __abs))
					{
						// _GLIBCXX_RESOLVE_LIB_DEFECTS
						// 2100.  timed waiting functions must also join
						// See wait_for(...) above.
						_M_complete_async();
						return future_status::ready;
					}
					return future_status::timeout;
				}
			// Provide a result to the shared state and make it ready.
			// Calls at most once: _M_result = __res();
			void
				_M_set_result(function<_Ptr_type()> __res, bool __ignore_failure = false)
				{
					bool __did_set = false;
					// all calls to this function are serialized,
					// side-effects of invoking __res only happen once
					call_once(_M_once, &_State_baseV2::_M_do_set, this,
							std::__addressof(__res), std::__addressof(__did_set));
					if (__did_set)
						// Use release MO to synchronize with observers of the ready state.
						_M_status._M_store_notify_all(_Status::__ready,
								memory_order_release);
					else if (!__ignore_failure)
						__throw_future_error(int(future_errc::promise_already_satisfied));
				}
			// Provide a result to the shared state but delay making it ready
			// until the calling thread exits.
			// Calls at most once: _M_result = __res();
			void
				_M_set_delayed_result(function<_Ptr_type()> __res,
						weak_ptr<_State_baseV2> __self)
				{
					bool __did_set = false;
					unique_ptr<_Make_ready> __mr{new _Make_ready};
					// all calls to this function are serialized,
					// side-effects of invoking __res only happen once
					call_once(_M_once, &_State_baseV2::_M_do_set, this,
							std::__addressof(__res), std::__addressof(__did_set));
					if (!__did_set)
						__throw_future_error(int(future_errc::promise_already_satisfied));
					__mr->_M_shared_state = std::move(__self);
					__mr->_M_set();
					__mr.release();
				}
			// Abandon this shared state.
			void
				_M_break_promise(_Ptr_type __res)
				{
					if (static_cast<bool>(__res))
					{
						__res->_M_error =
							make_exception_ptr(future_error(future_errc::broken_promise));
						// This function is only called when the last asynchronous result
						// provider is abandoning this shared state, so noone can be
						// trying to make the shared state ready at the same time, and
						// we can access _M_result directly instead of through call_once.
						_M_result.swap(__res);
						// Use release MO to synchronize with observers of the ready state.
						_M_status._M_store_notify_all(_Status::__ready,
								memory_order_release);
					}
				}
			// // Called when this object is first passed to a future.
			// void
			// 	_M_set_retrieved_flag()
			// 	{
			// 		if (_M_retrieved.test_and_set())
			// 			__throw_future_error(int(future_errc::future_already_retrieved));
			// 	}

			// template<typename _Res, typename _Arg>
				// struct _Setter;
			// set lvalues
			// template<typename _Res, typename _Arg>
			// 	struct _Setter<_Res, _Arg&>
			// 	{
			// 		// check this is only used by promise<R>::set_value(const R&)
			// 		// or promise<R&>::set_value(R&)
			// 		static_assert(is_same<_Res, _Arg&>::value  // promise<R&>
			// 				|| is_same<const _Res, _Arg>::value,   // promise<R>
			// 				"Invalid specialisation");
			// 		// Used by std::promise to copy construct the result.
			// 		typename promise<_Res>::_Ptr_type operator()() const
			// 		{
			// 			_M_promise->_M_storage->_M_set(*_M_arg);
			// 			return std::move(_M_promise->_M_storage);
			// 		}
			// 		promise<_Res>*    _M_promise;
			// 		_Arg*             _M_arg;
			// 	};
			// // set rvalues
			// template<typename _Res>
			// 	struct _Setter<_Res, _Res&&>
			// 	{
			// 		// Used by std::promise to move construct the result.
			// 		typename promise<_Res>::_Ptr_type operator()() const
			// 		{
			// 			_M_promise->_M_storage->_M_set(std::move(*_M_arg));
			// 			return std::move(_M_promise->_M_storage);
			// 		}
			// 		promise<_Res>*    _M_promise;
			// 		_Res*             _M_arg;
			// 	};
			// // set void
			// template<typename _Res>
			// 	struct _Setter<_Res, void>
			// 	{
			// 		static_assert(is_void<_Res>::value, "Only used for promise<void>");
			// 		typename promise<_Res>::_Ptr_type operator()() const
			// 		{ return std::move(_M_promise->_M_storage); }
			// 		promise<_Res>*    _M_promise;
			// 	};
			// struct __exception_ptr_tag { };
			// // set exceptions
			// template<typename _Res>
			// 	struct _Setter<_Res, __exception_ptr_tag>
			// 	{
			// 		// Used by std::promise to store an exception as the result.
			// 		typename promise<_Res>::_Ptr_type operator()() const
			// 		{
			// 			_M_promise->_M_storage->_M_error = *_M_ex;
			// 			return std::move(_M_promise->_M_storage);
			// 		}
			// 		promise<_Res>*   _M_promise;
			// 		exception_ptr*    _M_ex;
			// 	};
			// template<typename _Res, typename _Arg>
			// 	static _Setter<_Res, _Arg&&>
			// 	__setter(promise<_Res>* __prom, _Arg&& __arg)
			// 	{
			// 		_S_check(__prom->_M_future);
			// 		return _Setter<_Res, _Arg&&>{ __prom, std::__addressof(__arg) };
			// 	}
			// template<typename _Res>
			// 	static _Setter<_Res, __exception_ptr_tag>
			// 	__setter(exception_ptr& __ex, promise<_Res>* __prom)
			// 	{
			// 		_S_check(__prom->_M_future);
			// 		return _Setter<_Res, __exception_ptr_tag>{ __prom, &__ex };
			// 	}
			// template<typename _Res>
			// 	static _Setter<_Res, void>
			// 	__setter(promise<_Res>* __prom)
			// 	{
			// 		_S_check(__prom->_M_future);
			// 		return _Setter<_Res, void>{ __prom };
			// 	}

			// template<typename _Tp>
			// 	static void
			// 	_S_check(const shared_ptr<_Tp>& __p)
			// 	{
			// 		if (!static_cast<bool>(__p))
			// 			__throw_future_error((int)future_errc::no_state);
			// 	}
			private:
			// The function invoked with std::call_once(_M_once, ...).
			void
				_M_do_set(function<_Ptr_type()>* __f, bool* __did_set)
				{
					_Ptr_type __res = (*__f)();
					// Notify the caller that we did try to set; if we do not throw an
					// exception, the caller will be aware that it did set (e.g., see
					// _M_set_result).
					*__did_set = true;
					_M_result.swap(__res); // nothrow
				}
			// Wait for completion of async function.
			virtual void _M_complete_async() { }
			// Return true if state corresponds to a deferred function.
			virtual bool _M_is_deferred_future() const { return false; }
			// struct _Make_ready final : __at_thread_exit_elt
			// {
			// 	weak_ptr<_State_baseV2> _M_shared_state;
			// 	static void _S_run(void*);
			// 	void _M_set();
			// };
		};
	};


	/* "__basic_future" related */
	// using _State_base = _State_baseV2;
	template<typename _Res>
	class __basic_future : public _future_base
	{
		protected:
			typedef shared_ptr<_State_base>              __state_type;
			typedef __future_base::_Result<_Res>&        __result_type;
		private:
			__state_type                 _M_state;
		public:
			// Disable copying.
			__basic_future(const __basic_future&) = delete;
			__basic_future& operator=(const __basic_future&) = delete;

			bool
				valid() const noexcept { return static_cast<bool>(_M_state); }
			void
				wait() const
				{
					_State_base::_S_check(_M_state);
					_M_state->wait();
				}
			template<typename _Rep, typename _Period>
				future_status
				wait_for(const chrono::duration<_Rep, _Period>& __rel) const
				{
					_State_base::_S_check(_M_state);
					return _M_state->wait_for(__rel);
				}
			template<typename _Clock, typename _Duration>
				future_status
				wait_until(const chrono::time_point<_Clock, _Duration>& __abs) const
				{
					_State_base::_S_check(_M_state);
					return _M_state->wait_until(__abs);
				}
		protected:

			/// Wait for the state to be ready and rethrow any stored exception
			__result_type
				_M_get_result() const
				{
					_State_base::_S_check(_M_state);
					_Result_base& __res = _M_state->wait();
					if (!(__res._M_error == 0))
						rethrow_exception(__res._M_error);
					return static_cast<__result_type>(__res);
				}

			void _M_swap(__basic_future& __that) noexcept
			{
				_M_state.swap(__that._M_state);
			}

			// Construction of a future by promise::get_future()
			explicit
				__basic_future(const __state_type& __state) : _M_state(__state)
			{
				_State_base::_S_check(_M_state);
				_M_state->_M_set_retrieved_flag();
			}

			// // Copy construction from a shared_future
			// explicit
			// 	__basic_future(const shared_future<_Res>&) noexcept;

			// // Move construction from a shared_future
			// explicit
			// 	__basic_future(shared_future<_Res>&&) noexcept;

			// Move construction from a future
			// explicit
			// 	__basic_future(future<_Res>&&) noexcept;

			constexpr __basic_future() noexcept : _M_state() { }

			struct _Reset
			{
				explicit _Reset(__basic_future& __fut) noexcept : _M_fut(__fut) { }
				~_Reset() { _M_fut._M_state.reset(); }
				__basic_future& _M_fut;
			};
	};




	/* "class future" related */
	/** Primary template for future. **/
	template<typename _Res>
	class future : public __basic_future<_Res>
	{
		// friend class promise<_Res>;
		// template<typename> friend class packaged_task;

		template<typename _Fn, typename... _Args>
		friend future<__async_result_of<_Fn, _Args...>>
		async(launch, _Fn&&, _Args&&...);

		typedef __basic_future<_Res> _Base_type;
		typedef typename _Base_type::__state_type __state_type;
		explicit
		future(const __state_type& __state) : _Base_type(__state) { }

		public:
		constexpr future() noexcept : _Base_type() { }

		/// Move constructor
		future(future&& __uf) noexcept : _Base_type(std::move(__uf)) { }

		// Disable copying
		future(const future&) = delete;
		future& operator=(const future&) = delete;

		future& operator=(future&& __fut) noexcept
		{
			future(std::move(__fut))._M_swap(*this);
			return *this;
		}



		/// Retrieving the value
		_Res
			get()
			{
				typename _Base_type::_Reset __reset(*this);
				return std::move(this->_M_get_result()._M_value());
			}

		// shared_future<_Res> share() noexcept;

	};
  
  
  // /** Partial specialization for future<R&> **//
  // template<typename _Res>
  //   class future<_Res&> : public __basic_future<_Res&>
  //   {
  //     // friend class promise<_Res&>;
  //     // template<typename> friend class packaged_task;
  //
  //     template<typename _Fn, typename... _Args>
  //       friend future<__async_result_of<_Fn, _Args...>>
  //       async(launch, _Fn&&, _Args&&...);
  //
  //     typedef __basic_future<_Res&> _Base_type;
  //     typedef typename _Base_type::__state_type __state_type;
  //
  //     explicit
  //     future(const __state_type& __state) : _Base_type(__state) { }
  //   public:
  //     constexpr future() noexcept : _Base_type() { }
	//   
  //     /// Move constructor
  //     future(future&& __uf) noexcept : _Base_type(std::move(__uf)) { }
	//  
  //     // Disable copying
  //     future(const future&) = delete;
  //     future& operator=(const future&) = delete;
  //
  //     future& operator=(future&& __fut) noexcept
  //     {
  //       future(std::move(__fut))._M_swap(*this);
  //       return *this;
  //     }
	//   
  //     /// Retrieving the value
  //     _Res& get()
  //     {
  //       typename _Base_type::_Reset __reset(*this);
  //       return this->_M_get_result()._M_get();
  //     }
	//   
  //     // shared_future<_Res&> share() noexcept;
  //   };
  
  
  /** Explicit specialization for future<void> **/
  // template<>
	// class future<void> : public __basic_future<void>
	// {
	// 	// friend class promise<void>;
	// 	template<typename> friend class packaged_task;
	// 	template<typename _Fn, typename... _Args>
	// 		friend future<__async_result_of<_Fn, _Args...>>
	// 		async(launch, _Fn&&, _Args&&...);
  //
	// 	typedef __basic_future<void> _Base_type;
	// 	typedef typename _Base_type::__state_type __state_type;
  //
	// 	explicit
	// 		future(const __state_type& __state) : _Base_type(__state) { }
	// 	public:
	// 	constexpr future() noexcept : _Base_type() { }
	// 	
	// 	/// Move constructor
	// 	future(future&& __uf) noexcept : _Base_type(std::move(__uf)) { }
  //
	// 	// Disable copying
	// 	future(const future&) = delete;
	// 	future& operator=(const future&) = delete;
  //
	// 	future& operator=(future&& __fut) noexcept
	// 	{
	// 		future(std::move(__fut))._M_swap(*this);
	// 		return *this;
	// 	}
	// 	
	// 	
	// 	
	// 	/// Retrieving the value
	// 	void
	// 		get()
	// 		{
	// 			typename _Base_type::_Reset __reset(*this);
	// 			this->_M_get_result();
	// 		}
  //
	// 	// shared_future<void> share() noexcept;
  //   };







	/* "async" related */
	template<typename _Fn, typename... _Args>
	using __async_result_of = typename result_of<
	typename decay<_Fn>::type(typename decay<_Args>::type...)>::type;

	template<typename _Fn, typename... _Args>
	future<__async_result_of<_Fn, _Args...>>
	async(launch __policy, _Fn&& __fn, _Args&&... __args)
	{
		std::shared_ptr<__future_base::_State_base> __state;
		if ((__policy & launch::async) == launch::async)
		{
			__try
			{
				__state = __future_base::_S_make_async_state(
						std::thread::__make_invoker(std::forward<_Fn>(__fn),
							std::forward<_Args>(__args)...)
						);
			}
				#if __cpp_exceptions
				catch(const system_error& __e)
				{
					if (__e.code() != errc::resource_unavailable_try_again
							|| (__policy & launch::deferred) != launch::deferred)
						throw;
				}
				#endif
			}
			if (!__state)
			{
				__state = __future_base::_S_make_deferred_state(
						std::thread::__make_invoker(std::forward<_Fn>(__fn),
							std::forward<_Args>(__args)...));
			}
			return future<__async_result_of<_Fn, _Args...>>(__state);
		}
	/// async, potential overload
	template<typename _Fn, typename... _Args>
		inline future<__async_result_of<_Fn, _Args...>>
		async(_Fn&& __fn, _Args&&... __args)
		{
			return std::async(launch::async|launch::deferred,
					std::forward<_Fn>(__fn),
					std::forward<_Args>(__args)...);
		}
}
#endif



