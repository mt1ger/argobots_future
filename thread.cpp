#include "thread.h"

// stdx::thread::thread(void (* func)(void*), void *argu)
// {
// 	int rank;
// 	int flag;
//
// 	#<{(| Initializing pools, schedulors and ESs in singleton class |)}>#
// 	#<{(| And offer a handler to reach the resources for this ULT |)}>#
// 	psingleton = thread_Singleton::instance();
//
// 	ABT_xstream_self_rank(&rank);
// 	ABT_pool target_pool = psingleton->pools[rank]; 
// 	flag = ABT_thread_create(target_pool, func, argu,
// 			ABT_THREAD_ATTR_NULL, &__id.ult);
// 	tid = psingleton->Gtid;
// 	psingleton->Gtid++;
// }

stdx::thread::thread (thread&& other) 
{
	swap (other);
}


void 
stdx::thread::join ()
{
	if(__id != id())
		ABT_thread_free (&__id.ult);
	__id = id();
}

stdx::thread::id 
stdx::thread::get_id () 
const noexcept
{
	return this->__id;
}

bool
stdx::thread::joinable () 
{
	return !(__id == id());
}

void 
stdx::thread::swap(thread & other) 
{
	std::swap (this->__id, other.__id);
}

stdx::thread& 
stdx::thread::operator=(thread&& other)
{
	
	if (this->joinable())
	{
		std::terminate();
	}
	this->swap(other);
	return *this;
}

bool 
stdx::operator==(stdx::thread::id id1, stdx::thread::id id2) 
noexcept
{
	return id1.ult == id2.ult;	
}


ostream&
stdx::operator<< (ostream& __out, stdx::thread::id id1)
{
	if (id1 == stdx::thread::id())
		return __out << "thread::id of a non-executing thread";
	else
		return __out << id1.ult;
}
bool
stdx::operator!=(stdx::thread::id id1, stdx::thread::id id2)
noexcept
{ return !(id1 == id2);}
bool
stdx::operator<(stdx::thread::id id1, stdx::thread::id id2)
noexcept
{ return id1.ult < id2.ult;}
bool
stdx::operator<=(stdx::thread::id id1, stdx::thread::id id2)
noexcept
{ return !(id2 < id1);}
bool
stdx::operator>(stdx::thread::id id1, stdx::thread::id id2)
noexcept
{ return id2 < id1;}
bool
stdx::operator>=(stdx::thread::id id1, stdx::thread::id id2)
noexcept
{ return !(id1 < id2);}
