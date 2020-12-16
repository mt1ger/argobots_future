#ifndef _CLASS_PROMISE_CPP
#define _CLASS_PROMISE_CPP

#include"future.h"



template<class Ret, class ...Args>
template<class Fn>
stdx::packaged_task<Ret(Args...)>::packaged_task(Fn && func) noexcept
{
	ABT_eventual_create(0, &pt_eventual_);
}


#endif
