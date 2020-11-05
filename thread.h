#ifndef __THREAD_H
#define __THREAD_H

#include<abt.h>
#include<cstdlib>
#include<iostream>

#include "thread_Singleton.h"


namespace stdx 
{

	class thread
	{
		public:
			class id 
			{
				ABT_thread ult;
				public:
				id () noexcept : ult (){}
				id (ABT_thread other): ult(other) {}

				private:
					friend class thread;
					friend bool operator== (thread::id id1, thread::id id2) noexcept;
					friend bool operator< (thread::id id1, thread::id id2) noexcept;

					friend 
					ostream&
					operator<< (ostream& __out, thread::id id1);
			};
		public:

			thread_Singleton* psingleton;

			/* default constructor */
			thread () noexcept {}
			/* constructor with parameters */
			template<class Fn, class ...Args>
			thread(Fn func, Args ...args)
			{
				int rank;
				int flag;

				/* Initializing pools, schedulors and ESs in singleton class */
				/* And offer a handler to reach the resources for this ULT */
				psingleton = thread_Singleton::instance();

				ABT_xstream_self_rank(&rank);
				ABT_pool target_pool = psingleton->pools[rank]; 
				flag = ABT_thread_create(target_pool, func, args...,
						ABT_THREAD_ATTR_NULL, &__id.ult);
				tid = psingleton->Gtid;
				psingleton->Gtid++;
			}

			thread (thread&& other);
			thread (thread&) = delete;
			thread (const thread&) = delete;
			thread (const thread&&) = delete;

			~thread() {}

			void join ();
			void detach();
			bool joinable ();
			id get_id () const noexcept;
			void swap(thread & other);

			thread& operator=(thread&& other);

		private:
			id __id;
			ABT_thread_id tid;
	};

	ostream& operator<<(ostream& __out, thread::id id2);
	bool operator==(thread::id id1, thread::id id2) noexcept;
	bool operator<(thread::id id1, thread::id id2) noexcept;
	bool operator>(thread::id id1, thread::id id2) noexcept;
	bool operator!=(thread::id id1, thread::id id2) noexcept;
	bool operator>=(thread::id id1, thread::id id2) noexcept;
	bool operator<=(thread::id id1, thread::id id2) noexcept;
};
#endif


