#ifndef PROCESS_H
#define PROCESS_H

#include <iostream>
#include <string>

using namespace std;

class Process{
	private:
		int pid;
		int burst;
		int arrival;
		int priority;
		int deadline;
		int io;

		int start_time = 0;
		int age = 0;
		int start_burst = 0;
		int start_priority = 0;

	public:
		Process(int pid, int burst, int arrival, int priority, int deadline, int io);
		Process(){
			pid = 0;
			burst = 0;
			arrival = 0;
			priority = 0;
			deadline = 0;
			io = 0;		
		}
		
		int get_pid()const{return pid;}
		int get_burst()const{return burst;}
		int get_arrival()const{return arrival;}
		int get_priority()const{return priority;}
		int get_deadline()const{return deadline;}
		int get_io()const{return io;}
		int get_total_burst()const{return start_burst;}
		int get_start_time()const{return start_time;}
		int get_start_priority()const{return start_priority;}
		int get_age()const{return age;}
		void set_burst(int quantum){burst = burst - quantum;}
		void set_priority(int n){priority = n;}
		void premote_priority(int n){priority = priority + n;}
		void set_start_time(int n){start_time = n;}
		void set_total_burst(int n){start_burst = n;}
		void set_age(int n ){age = age + n;}
		void reset_age(){age = 0;}
		void set_start_priority(int n){start_priority = n;}
		void print_rts()const;
		void print_mfqs()const;
		void print_whs()const;
};
#endif
