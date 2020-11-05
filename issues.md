# the key lines
```c++
	__State_baseV2 (309, 581);
	class future (759,886);
	class __basic_future (671, 756);
	struct __future_base (199, 264);
```



# Errors

## DOES NOT HAVE ISSUE
## "Async" related: Line 202 
1. +6, +33: "future" is not defined.
2. +9, +30: "__future_base" is not defined.
	* __future_base::__State_baseV2
	* __future_base::_S_make_deferred_state
3. +12: "__try" is not defined.
4. +31: "std:thread::__make_invoker" is not defined.
	* make_tuple (), return a tuple
5. +41: async is not defined.
	* Because "async" at Line 72 is can not be defined. 


## DOES NOT HAVE ISSUE
## "class future" related: Line 70
1. +3, +11: "__basic_future" is not defined.
2. +9: "__async_result_of" is not defined.
3. +12: "_Base_type" is not defined.
	* "_Base_type" is "__basic_future" 
	* related to 1

# Requirement

## "Async" related
* "__async_result_of"
