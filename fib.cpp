#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <chrono>
#include <type_traits>
#include <functional>
// #include<ostream>

#include"future_test.h"
// #include"thread.h"

using namespace std;

typedef struct {
    int n;
    int ret;
} fibonacci_arg_t;


typedef struct 
{
	int ret; 	
} test1;


void fibonacci(void *arg)
{
    int n = ((fibonacci_arg_t *)arg)->n;
    int *p_ret = &((fibonacci_arg_t *)arg)->ret;

    if (n <= 1) {
        *p_ret = 1;
    } 
	else {
        fibonacci_arg_t child1_arg = {n - 1, 0};
        fibonacci_arg_t child2_arg = {n - 2, 0};

		stdx::thread threads (fibonacci, &child1_arg);

        /* Calculate fib(n - 2).  We do not create another ULT. */
        fibonacci(&child2_arg);

		threads.join();

        *p_ret = child1_arg.ret + child2_arg.ret;
    }
}

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


void as (void * argu) 
{
	int a = ((test1 *)argu)->ret;
	// int *p = &((test1 *)argu)->ret;
	printf("the value is %d\n", a * 50);
	sleep(2);
	((test1*)argu)->ret = a * 50;
	// *p = a * 50;
	// cout << ((test1*)argu)->ret  << endl;
}

typedef struct
{
	stdx::future<int> fut;
}stf;

void print_int (void * ptr) {
	stf* s_ptr = (stf*  ) ptr;
  int x = s_ptr->fut.get();
  std::cout << "value: " << x << '\n';
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

	chrono::steady_clock::time_point start = chrono::steady_clock::now();

	fibonacci_arg_t arg = {n, 0};
    fibonacci(&arg);
    int ret = arg.ret;
    int ans = fibonacci_seq(n);
	cout << "The returned value is " << ret << "; The verification is " << ans << endl;

	chrono::steady_clock::time_point end = chrono::steady_clock::now();
	chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double> >(end-start);
	cout << "Execution time: " << time_span.count() << endl;


	/* Do not touch this */
	test1 te1;
	te1.ret = 5;
	test1* te2;
	te2 = new test1;
	te2->ret = 15;
	test1* te3;

	/* used to test promise.set_value and future.get */
	// stf futu;
	// stdx::promise<int> foo;
	// stdx::promise<int> bar = stdx::promise<int>();
	// futu.fut = bar.get_future();
	// stdx::thread th (print_int, &futu);
	// bar.set_value (20);
	// th.join();
	// cout << futu.fut.get ();






	
	stdx::future<test1*> fut1;
	fut1 = stdx::async(as, &te1);
	test1 * test;
	fut1.wait();

	test = fut1.get();
	cout << "the value is: "<< test->ret << endl;
	// stdx::promise<test1> prom1;
	// fut1 = prom1.get_future();
	// prom1.set_value(te1);
	// test1 ttt;
	// ttt = fut1.get();
	// cout << ttt.ret << endl;
	


	
	// stdx::async (stdx::launch::async, as, te1);
	// t1 = stdx::thread (as, &te1);
	// ABT_eventual_set(ev1, &te1, sizeof(te1));
	// ABT_eventual_wait(ev1, (void**)&aaa);
	// test1 te2;
	// te2 = fut1.get ();
	printf ("te1 %d\n", te1.ret);
	printf ("te2 %d\n", te2->ret);

	return 1;
}

