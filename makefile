all:
	g++-10 -I /Users/mt1ger/argobots-install/include/ -labt -L /Users/mt1ger/argobots-install/lib/ fib.cpp class_future.cpp FuncInFuture.cpp thread.cpp thread_Singleton.cpp -o test
