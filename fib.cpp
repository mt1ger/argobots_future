#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <chrono>
#include <type_traits>
#include <functional>

#include<future>

#include"future.h"

// #define FUTURE
// #define XFUTURE
#define TEST_TIMES 100

using namespace std;

typedef struct {
    int n;
    int ret;
} fibonacci_arg_t;

#ifdef FUTURE
void fibonacci_future(void *arg)
{
    int n = ((fibonacci_arg_t *)arg)->n;
    int *p_ret = &((fibonacci_arg_t *)arg)->ret;

    if (n <= 1) {
        *p_ret = 1;
    } 
	else {
        fibonacci_arg_t child1_arg = {n - 1, 0};
        fibonacci_arg_t child2_arg = {n - 2, 0};


		std::future<void> fut;
		fut = std::async(std::launch::async, fibonacci_future, &child1_arg);


        /* Calculate fib(n - 2).  We do not create another ULT. */
        fibonacci_future(&child2_arg);

		// threads.join();
		fut.get();

        *p_ret = child1_arg.ret + child2_arg.ret;
    }
}
#endif

#ifdef XFUTURE
void fibonacci_xfuture(void *arg)
{
    int n = ((fibonacci_arg_t *)arg)->n;
    int *p_ret = &((fibonacci_arg_t *)arg)->ret;

    if (n <= 1) {
        *p_ret = 1;
    } 
	else {
        fibonacci_arg_t child1_arg = {n - 1, 0};
        fibonacci_arg_t child2_arg = {n - 2, 0};

		// stdx::thread threads (fibonacci, &child1_arg);
		stdx::future<void> fut;
		fut = stdx::async(stdx::launch::async, fibonacci_xfuture, &child1_arg); 

        /* Calculate fib(n - 2).  We do not create another ULT. */
        fibonacci_xfuture(&child2_arg);

		// threads.join();
		fut.get();

        *p_ret = child1_arg.ret + child2_arg.ret;
    }
}
#endif

int fibonacci_seq(int n)
{
    if (n <= 1) {
        return 1;
    } else {
        int i;
        int fib_i1 = 1; /* Value of fib(i - 1) */
        int fib_i2 = 1; /* Value of fib(i - 2) */
        for (i = 3; i <= n; i++) {
            int tmp = fib_i1;
            fib_i1 = fib_i1 + fib_i2;
            fib_i2 = tmp;
        }
        return fib_i1 + fib_i2;
    }
}


void void_func_voidp (void * argu) 
{
	sleep(3);
	int a = ((fibonacci_arg_t *)argu)->ret;
	// int *p = &((test1 *)argu)->ret;
	printf("the value is %d\n", a * 50);
	((fibonacci_arg_t *)argu)->ret = a * 50;
	// *p = a * 50;
	// cout << ((test1*)argu)->ret  << endl;
}


int int_func_in (int a) 
{
	sleep(3);
	cout << "in function what a is " << a << endl;
	return a;
}


int main (int argc, char * argv[])
{

	int n = 13;

	/* Read arguments. */
	while (1) {
		int opt = getopt(argc, argv, "he:n:");
		if (opt == -1)
			break;
		switch (opt) {
			case 'e':
				// num_xstreams = atoi(optarg);
				// cout << num_xstreams << " are needed." << endl;
				break;
			case 'n':
				n = atoi(optarg);
				break;
			case 'h':
			default:
				printf("Usage: ./stdThread [-e NUM_XSTREAMS]\n");
				return -1;
		}
	}

	fibonacci_arg_t args;
	chrono::steady_clock::time_point start = chrono::steady_clock::now();

	for (int i=0; i<TEST_TIMES; i++) 
	{
		args = {n, 0};
		/* For future */
		#ifdef FUTURE
		fibonacci_future(&args);
		#endif

		/* For xfuture */
		#ifdef XFUTURE
		fibonacci_xfuture(&args);
		#endif
	}

	chrono::steady_clock::time_point end = chrono::steady_clock::now();
	chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double> >(end-start);
	cout << "Execution time: " << time_span.count() << endl;
	cout << "Execution time: " << time_span.count() / 100 << endl;
	int ret = args.ret;
	int ans = fibonacci_seq(n);
	cout << "The returned value is " << ret << "; The verification is " << ans << endl;

	chrono::milliseconds ms (5000);
	chrono::steady_clock::time_point e = chrono::steady_clock::now();
	chrono::steady_clock::time_point s = chrono::steady_clock::now();
	

	/* Do not touch this */
	fibonacci_arg_t te1;
	te1.ret = 5;
	fibonacci_arg_t * te2;
	te2 = new fibonacci_arg_t;
	te2->ret = 15;

	/* used to test promise.set_value and future.get */
	// stf futu;
	// // stdx::promise<int> foo;
	// stdx::promise<int> bar = stdx::promise<int>();
	// futu.fut = bar.get_future();
	// stdx::thread th (print_int, &futu);
	// stdx::thread th ([](void* ptr){stdx::promise<int> * ptr_internal = (stdx::promise<int>*) ptr; ptr_internal->set_value_at_thread_exit(50);}, &bar);
	// // stdx::thread th ([&bar](){bar.set_value_at_thread_exit(50);});
	// // // bar.set_value (150);
	// // // th.join();
	// futu.fut.wait();
	// cout << futu.fut.get ();

	/* Use to Test ABT_eventual Functions */
	// int flag;
	// int wait_flag;
	// ABT_eventual ev1;
	// ABT_eventual ev2;
	// event_related ev_struct1;
	// event_related ev_struct2;
	// ABT_eventual_create(0, &ev1);
	// ABT_eventual_create(0, &ev2);
	// // ABT_eventual_test(event, nullptr, &flag) ;
	// // cout << "the value is " << flag << endl;
	// ev_struct1.ev = ev1;
	// // ev_struct2.ev = ev2;
	// ev_struct2.ev = ev1;

	// stdx::thread ([](int x)
	// 	{
	// 	cout <<
	// 		x*x << endl;
	// 	},
	// 	7
	// );
	// stdx::thread(wake_up, &ev_struct2);
	// ABT_eventual_test(event, nullptr, &flag) ;
	// cout << "the value is " << flag << endl;
	// ABT_eventual_wait(ev1, nullptr);
	// ABT_eventual_test(event, nullptr, &flag) ;
	// cout << "the value is " << flag << endl;
	// ABT_eventual_reset(ev1);
	// ABT_eventual_test(event, nullptr, &flag) ;
	// cout << "the value is " << flag << endl;
	// cout << "finished" << endl;


	/* section to test wait(), wait_for() and wait_until() */
	// chrono::milliseconds span (100);
	// std::chrono::steady_clock::time_point two_seconds_passed
	// 	= std::chrono::steady_clock::now() + std::chrono::seconds(3);
	// stdx::future<void> fut1;
	// stdx::future<void> fut3;
	// stdx::future<int> fut2;
	stdx::future<int> fut4;
	// int a1 = 50;
	int a2 = 333;
	// fut1 = stdx::async(stdx::launch::async, void_func_voidp, &te1);
	// fut3 = stdx::async(stdx::launch::deferred, void_func_voidp, te2);
	// fut2 = stdx::async(stdx::launch::async, int_func_in, a1);
	fut4 = stdx::async(stdx::launch::deferred, int_func_in, a2);

	// while(stdx::future_status::timeout == fut2.wait_until(two_seconds_passed))
	// while(stdx::future_status::timeout == fut1.wait_until(two_seconds_passed))
	// 	cout << '.';
	// cout << endl;

	// if(stdx::future_status::ready == fut1.wait_until(two_seconds_passed))
	// if(stdx::future_status::ready == fut2.wait_until(two_seconds_passed))
	// {
	// 	std::cout << "f_completes: " << fut2.get() << "\n"; 
	// 	// std::cout << "f_completes " << endl; 
	// 	// fut1.get(); 
	// }
	// else
	// { std::cout << "f_completes did not complete!\n"; }

	// fut1.wait();
	// fut3.wait();
	// test = fut1.get();
	// fut1.get();
	// fut3.get();

	// fut2.wait ();
	// fut4.wait ();
	int  ret1 = 0, ret2 = 0;
	// ret1 = fut2.get();
	// ret2 = fut4.get();
	cout << "the ret is " << ret1 << " and " << ret2 << endl;

	return 1;
}

