#ifndef __SINGLETON_H
#define __SINGLETON_H

#include<cstdlib>
#include<iostream>
#include<abt.h>

using namespace std;

class thread_Singleton 
{

	int num_xstreams = 0;	
	static thread_Singleton * tsingleton_ptr;
	ABT_xstream * xstreams;
	ABT_sched* scheds; 

	thread_Singleton()
	{
		Gtid = 0;
		num_xstreams = atoi(getenv("NUM_ES"));
	    ABT_init(0, nullptr);
		mem_allocation ();
		pools_scheds_creation ();
		main_xstream ();
		secondary_xstreams ();
	}

	~thread_Singleton()
	{
		join_free_xstream();
		finalize();
	}


	void mem_allocation (); 
	void pools_scheds_creation ();
	void main_xstream ();
	void secondary_xstreams ();
	void join_free_xstream ();
	void finalize ();


	public:
	static thread_Singleton* instance ();
	ABT_pool* pools; 
	ABT_thread_id Gtid;
};
#endif
