#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <errno.h>
#include <chrono>
#include <stdexcept>
#include <array>
#include <math.h>
#include <vector>
#include <sstream>
#include <algorithm>

#include "process.h"

#define OUTPUT

using namespace std;
using namespace chrono;

void runMFQS(int num_q);
void runRTS();
void runWHS();
void demote_whs(Process p, int band, int quantum);
bool compare_priority(Process p1, Process p2);
bool compare_priority_whs(Process p1, Process p2);
bool compare_arrival(Process p1, Process p2);
bool compare_deadline(Process p1, Process p2);
bool more_processes(vector<Process> v1, vector<Process> v2);
int incoming_p(Process p, int clock, int new_p);
void check_age(vector<Process> p , int quantum, int num_p, int ct);

int main(int argc, char* argv[]){
	cout << "***********************************************\n";
	cout << "               Process schduling               \n";
	cout << "***********************************************\n";

	int scheduler_type;
	cout << "Enter the number for which algorithm to run:\n";
	cout << "\t1 for MFQS\n\t2 for RTS\n\t3 for WHS\n";

	cin >> scheduler_type;

	switch(scheduler_type){
		case 1:{
			int num_q;
			cout << "Running MFQS\n";
			cout << "Enter number of queues from 1-5\n";
			cin >> num_q;
			runMFQS(num_q);
		break;
		}
		case 2:{
			cout << "Running RTS\n";
			runRTS();
		break;
		}
		case 3:{
			cout << "Running WHS\n";
			runWHS();
		break;
		}
	}
}
void runWHS(){
	int pid, burst, arrival, priority, deadline, io;
	int clock_tick = 0;
	double process;
	vector<Process> processes;
	
	cout << "Enter file name to simulate:\n";
	string file_name;
	cin >> file_name;
	
	cout << "Enter the time quantum: \n";
	int quantum;
	cin >> quantum;
 
	ifstream file;
	file.open(file_name);

	string line;
	getline(file, line);
	//get all valid processes for file and sort by arrival time
	while(getline(file, line)){
		
		istringstream ss(line);
		ss >> pid >> burst >> arrival >> priority >> deadline >> io;
		
		if(pid > 0 && burst >= 0 && arrival >= 0 && priority >= 0 && io >=0){
			//cout << line << endl;
			Process pro(pid, burst, arrival, priority, deadline, io);
			pro.set_start_time(0);
			pro.set_start_priority(pro.get_priority());
			processes.push_back(pro);
		}
	}
	file.close();
	sort(processes.begin(),processes.end(), compare_arrival);
	clock_tick = processes.back().get_arrival();
	
	double total_waiting = 0;
	double total_turnaround = 0;
	int num_p = processes.size();
	int num_high_p = 0;
	int num_low_p = 0;
	process = processes.size();
	vector<Process> highband;
	vector<Process> lowband;
	while(num_p > 0){
		
	
		if(processes.size() > 0){
			Process p = processes.back();
			if(p.get_arrival() <= clock_tick){
				if(p.get_priority() > 50){
					highband.push_back(processes.back());
					processes.pop_back();
#ifndef OUTPUT
					cout << "Process: " << p.get_pid() << " arrived and moved to highband\n";
#endif
					num_high_p++;
				}else if(p.get_priority() < 50){
					lowband.push_back(processes.back());
					processes.pop_back();
#ifndef OUTPUT
					cout << "Process: " << p.get_pid() << " arrived moved to lowband\n";
#endif
					num_low_p++;
				}else{
#ifndef OUTPUT
					cout << "Process: " << p.get_pid() << " priortiy not in range removing...\n";
#endif
					processes.pop_back();
					num_p--;
				}
				
			}
		}		
		sort(lowband.begin(), lowband.end(), compare_priority_whs);
		sort(highband.begin(), highband.end(), compare_priority_whs);
		if(num_high_p > 0){
			Process curr_p = highband.back();
			highband.pop_back();
			//burst less than time quantum
			if(curr_p.get_burst() <= quantum){
				clock_tick = clock_tick + curr_p.get_burst();
				check_age(highband, curr_p.get_burst(), num_high_p, clock_tick);
#ifndef OUTPUT
				cout << "Process finished in highband.... PID: " << curr_p.get_pid()  << " at clock tick: " << clock_tick << endl;
#endif

				total_turnaround += clock_tick - curr_p.get_arrival();
				total_waiting += clock_tick - curr_p.get_arrival() - curr_p.get_total_burst();
				num_p--;
				num_high_p--;
			}else{//Burst is greater than time quantum
				//Do i/o
				if(curr_p.get_io() > 0){//NOT WORKING!!!!!!!!!!!!!!!!!!
					curr_p.set_burst(quantum - 1);
					clock_tick = clock_tick + quantum - 1;
						//Do I/O
					curr_p.premote_priority(curr_p.get_io());
#ifndef OUTPUT
					cout << "Process running highband..... PID: " << curr_p.get_pid()<< " at clock tick: " << clock_tick << endl;
#endif
					//cout << "\tI/O running for " << curr_p.get_io() << " clock ticks\n";
					clock_tick = clock_tick + curr_p.get_io();
#ifndef OUTPUT
					cout << "\tI/O finished at clock tick: " << clock_tick << endl;
#endif

			


				}else{//no i/o
					curr_p.set_burst(quantum);
					clock_tick = clock_tick + quantum;
					check_age(highband, curr_p.get_burst(), num_high_p, clock_tick);
#ifndef OUTPUT
					cout << "Process running highband..... PID: " << curr_p.get_pid()<< " at clock tick: " << clock_tick << endl;
#endif
	
					//Demote process and lower priority	
					if(curr_p.get_priority() - quantum > curr_p.get_start_priority()){
						if(curr_p.get_priority() - quantum < 50){
							curr_p.set_priority(50);
						}else{
							curr_p.set_priority(curr_p.get_priority()-quantum);
						}
					}
					curr_p.reset_age();
#ifndef OUTPUT
					cout << "Process demoted..... PID: " << curr_p.get_pid() << " new priority: " << curr_p.get_priority() << endl;
#endif
				}			
				
				highband.push_back(curr_p);
			}
		}else if(num_low_p > 0){
			
			Process curr = lowband.back();
			lowband.pop_back();
			//burst less than time quantum
			if(curr.get_burst() <= quantum){
				clock_tick = clock_tick + curr.get_burst();
				check_age(lowband, curr.get_burst(), num_low_p, clock_tick);
#ifndef OUTPUT
				cout << "Process finished in lowband.... PID: " << curr.get_pid()  << " at clock tick: " << clock_tick << endl;
#endif
				total_turnaround += clock_tick - curr.get_arrival();
				total_waiting += clock_tick - curr.get_arrival() - curr.get_total_burst();
				num_p--;
				num_low_p--;
			}else{//Burst is greater than time quantum
				if(curr.get_io() > 0){//NOT WORKING!!!!!!!!!!!!!!!!!!
					curr.set_burst(quantum - 1);
					clock_tick = clock_tick + quantum - 1;
						//Do I/O
					clock_tick = clock_tick + curr.get_io();
					curr.premote_priority(curr.get_io());
#ifndef OUTPUT
					cout << "Process running lowband..... PID: " << curr.get_pid()<< " at clock tick: " << clock_tick << endl;
#endif
					clock_tick = clock_tick + curr.get_io();
#ifndef OUTPUT
					cout << "\tI/O running for " << curr.get_io() << " clock ticks\n";
#endif
					//Demote process and lower priority
					if(curr.get_priority() - quantum > curr.get_start_priority()){
						if(curr.get_priority() - quantum < 0){
							curr.set_priority(0);
						}else{
							curr.set_priority(curr.get_priority()-quantum);
						}
					}


				}else{
					curr.set_burst(quantum);
					clock_tick = clock_tick + quantum;
#ifndef OUTPUT
					cout << "Process running lowband..... PID: " << curr.get_pid()<< " at clock tick: " << clock_tick << endl;
#endif
					//Demote process and lower priority
					if(curr.get_priority() - quantum > curr.get_start_priority()){
						if(curr.get_priority() - quantum < 0){
							curr.set_priority(0);
						}else{
							curr.set_priority(curr.get_priority()-quantum);
						}
					}
					curr.reset_age();
#ifndef OUTPUT
					cout << "Process demoted..... PID: " << curr.get_pid() << " new priority: " << curr.get_priority() << endl;
#endif
				}
				
				lowband.push_back(curr);
			}	
		}else{
			clock_tick++;
#ifndef OUTPUT
			cout << "No processes running at clock tick: " << clock_tick << endl;
#endif
		}
		check_age(highband , quantum, num_high_p, clock_tick);
		check_age(lowband , quantum, num_low_p, clock_tick);
		
	}
	cout << "\nTotal wait time: " << total_waiting << endl;
	cout << "Total turnaround: " << total_turnaround << endl;
	cout << "Average wait time: " << total_waiting/process << endl;
	cout << "Average turnaround: " << total_turnaround/process << endl;

}

void check_age(vector<Process> p , int quantum, int num_p, int ct){
	// ages bottom 10% of queue
	for(unsigned int i = p.size() - (p.size()/10); i < p.size(); i++){
		p[i].set_age(quantum);
	}
	//checks for aged processes to promote
	for(unsigned int i = p.size(); i < p.size(); i++){
		if(p[i].get_age() >= ct){
			cout << "Process premoted.... PID:" << p[i].get_pid() << endl;
			p[i].premote_priority(10);
		}
	}
	
}

void runMFQS(int num_q){
	int pid, burst, arrival, priority, deadline, io;
	int clock_tick = 0;
	double process;
	int con_switch = 0;
	vector<Process> processes;
	vector<vector<Process>> mfqs_q(num_q);
	
	cout << "Enter file name to simulate:\n";
	string file_name;
	cin >> file_name;

	cout << "Enter time quantum: \n";
	int quantum;
	cin >> quantum;

	cout << "Enter aging interval: \n";
	int age_interval;
	cin >> age_interval;

	ifstream file;
	file.open(file_name);

	string line;
	getline(file, line);
	//get all valid processes for file and sort by arrival time
	while(getline(file, line)){
		
		istringstream ss(line);
		ss >> pid >> burst >> arrival >> priority >> deadline >> io;
		
		if(pid > 0 && burst >= 0 && arrival >= 0 && priority >= 0){
			//cout << line << endl;
			Process pro(pid, burst, arrival, priority, deadline,0);
			pro.set_start_time(0);
			processes.push_back(pro);
		}
	}
	file.close();
	sort(processes.begin(),processes.end(), compare_arrival);
	//Sets the clock to first process
	clock_tick = processes.back().get_arrival();
	
/*
	for(unsigned int i = 0; i < processes.size(); i++){
		processes.at(i).print();
	}
*/
	double total_waiting = 0;
	double total_turnaround = 0;
	int total_p = processes.size();
	int new_p = processes.size();
	process = processes.size();
	
	while(total_p > 0){
		if(processes.size() > 0){
			Process p = processes.back();
			if(p.get_arrival() <= clock_tick){
				p.set_start_time(clock_tick);
				p.set_total_burst(p.get_burst());
				mfqs_q[0].push_back(p);
				processes.pop_back();
				new_p--;
			}
		}
		//Aging check in FCFS Queue
		for(unsigned int j = 0; j < mfqs_q[num_q-1].size();j++){
					mfqs_q[num_q-1][j].set_age(quantum);
					if(mfqs_q[num_q-1][j].get_age() == age_interval){
						mfqs_q[num_q-2].push_back(mfqs_q[num_q-1].back());
						mfqs_q[num_q-1].pop_back();
					}
				}

		for(int i = 0; i < num_q; i++){
			int curr_quantum = quantum;
			if(i != 0){
				curr_quantum = quantum * (i * 2);
			}
			Process curr_p;
			sort(mfqs_q[i].begin(), mfqs_q[i].end(), compare_priority);
			//enter rr queue 
			if(mfqs_q[i].size() > 0 && i != num_q -1){
				curr_p = mfqs_q[i].back();
				mfqs_q[i].pop_back();
			
#ifndef OUTPUT
				cout << "Process running..... PID: " << curr_p.get_pid()<< " in queue: " << i+1<< " at clock tick: " << clock_tick << endl;
#endif
				//Burst is less than time quantum
				if(curr_p.get_burst() - curr_quantum <= 0){
					clock_tick = clock_tick + curr_p.get_burst();
					con_switch++;
					total_p--;
#ifndef OUTPUT
					cout << "Process finished.... PID: " << curr_p.get_pid() << " in queue: " << i+1 << " at clock tick: " << clock_tick << endl;
#endif
					total_turnaround += clock_tick - curr_p.get_arrival();
					total_waiting += clock_tick - curr_p.get_arrival() - curr_p.get_total_burst();
					//cout << "Current quantum: " << curr_quantum << endl;
					//New process waiting in first queue check
					if(incoming_p(processes.back(), clock_tick, new_p))break;
					
	
				}else{//Burst is greater than time quantum
					curr_p.set_burst(curr_quantum);
					clock_tick = clock_tick + curr_quantum;
					con_switch++;
#ifndef OUTPUT				
					cout << "Process demoted..... PID: " << curr_p.get_pid() << " to queue: " << i+2 << endl;
#endif
					//cout << "Current quantum: " << curr_quantum << "burst: " << curr_p.get_burst() << endl;
					//New process waiting in first queue check
					mfqs_q[i+1].push_back(curr_p);
					//cout << mfqs_q[i+1].size()-1;
					if(mfqs_q[i+1].size() - 1  == 0)break;	
					if(incoming_p(processes.back(), clock_tick, new_p))break;
					//Demote process
					
				}
			//enter fcfs
			}else if(mfqs_q[i].size() > 0){
				curr_p = mfqs_q[i].back();
				mfqs_q[i].pop_back();
				clock_tick = clock_tick + curr_p.get_burst();
				con_switch++;
				total_p--;
				//cout << mfqs_q[i].size() << endl;
#ifndef OUTPUT
				cout << "Process finished.... PID: " << curr_p.get_pid() << " in queue: " << i+1 << " at clock tick: " << clock_tick << endl;
#endif
				total_turnaround += clock_tick - curr_p.get_arrival();
				total_waiting += clock_tick - curr_p.get_arrival() - curr_p.get_total_burst();
				break;
			
			}
		}
	}
	cout << "\nTotal wait time: " << total_waiting << endl;
	cout << "Total turnaround: " << total_turnaround << endl;
	cout << "Average wait time: " << total_waiting/process << endl;
	cout << "Average turnaround: " << total_turnaround/process << endl;
	cout << "Total context switches: " << con_switch << endl;

}

void runRTS(){

	int pid, burst, arrival, priority, deadline, io;
	int clock_tick = 0;
	int hard = 0;
	double process;
	int con_switch = 1;
	vector<Process> processes;
	vector<Process> rts_q;
	Process switch_pro;
	
	cout << "Enter file name to simulate:\n";
	string file_name;
	cin >> file_name;
	
	cout << "Enter hard(1) or soft(2) scheduler:\n";
	cin >> hard;

	ifstream file;
	file.open(file_name);

	string line;
	getline(file, line);

	while(getline(file, line)){
		
		istringstream ss(line);
		ss >> pid >> burst >> arrival >> priority >> deadline >> io;
		
		if(pid > 0 && burst >= 0 && arrival >= 0 && deadline >= 0){
			//cout << line << endl;
			Process pro(pid, burst, arrival, priority, deadline, 0);
			pro.set_start_time(0);
			pro.set_total_burst(pro.get_burst());
			processes.push_back(pro);
		}
	}
	file.close();
	sort(processes.begin(),processes.end(), compare_arrival);
	//Sets the clock to first process
	clock_tick = processes.back().get_arrival();
	
/*
	for(unsigned int i = 0; i < processes.size(); i++){
		processes.at(i).print();
	}
*/
	int total_p = processes.size();
	process = processes.size();
	double total_waiting = 0;
	double total_turnaround = 0;

	while(total_p != 0){
		//cout << "RUNNING" << total_p <<endl;
		//Move processes into rts queue when at its arrival time
		if(processes.size() > 0){
			Process p = processes.back();
			int met_deadline = p.get_deadline() - p.get_arrival() - p.get_burst();
			if(met_deadline <= 0){
#ifndef OUTPUT
				cout << "Unabable to finish process.... PID: " << p.get_pid() << endl;
#endif
				total_turnaround += clock_tick - p.get_arrival();
				total_waiting += clock_tick - p.get_arrival() - p.get_total_burst();
				if(hard == 1){
					cout << "Hard Real Time selected end simulation\n";
					exit(1);
				}else{
					processes.pop_back();
					p = processes.back();
					total_p--;
				}
			}else if(p.get_arrival() <= clock_tick){
				p.set_start_time(clock_tick);
				rts_q.push_back(p);
				processes.pop_back();
				sort(rts_q.begin(),rts_q.end(), compare_deadline);	
			}
		}
		//Runs the process with closest deadline	
		if(!rts_q.empty()){
			Process curr_p = rts_q.back();
			rts_q.pop_back();
			if(curr_p.get_pid() != switch_pro.get_pid()){
				con_switch++;
			}

			if(curr_p.get_burst() > 1){
				curr_p.set_burst(1);
				rts_q.push_back(curr_p);
#ifndef OUTPUT
				cout << "Process running..... PID:" << curr_p.get_pid() << " at clock tick: " << clock_tick << endl;
#endif
			}else if(curr_p.get_burst() <= 1){
#ifndef OUTPUT
				cout << "Process finished.... PID:" << curr_p.get_pid() << " at clock tick: " << clock_tick << endl;
#endif
				total_turnaround += clock_tick - curr_p.get_arrival();
				total_waiting += clock_tick - curr_p.get_arrival() - curr_p.get_total_burst();
				total_p--;
			}
			switch_pro = curr_p;
		}
		
		clock_tick++;
		
	}

	cout << "\nTotal wait time: " << total_waiting << endl;
	cout << "Total turnaround: " << total_turnaround << endl;
	cout << "Average wait time: " << total_waiting/process << endl;
	cout << "Average turnaround: " << total_turnaround/process << endl;
	cout << "Total context switches: " << con_switch << endl;
}

int incoming_p(Process p, int clock, int new_p){
	if(p.get_arrival() < clock && new_p != 0){
		return 1;
	}else{
		return 0;
	}
}
bool compare_priority(Process p1, Process p2){
	if(p1.get_priority() == p2.get_priority()) {
		return p1.get_pid() > p2.get_pid();
	}
	else{
		return p1.get_priority() > p2.get_priority();
	}
}

bool compare_priority_whs(Process p1, Process p2){
	if(p1.get_priority() == p2.get_priority()) {
		return p1.get_pid() < p2.get_pid();
	}
	else{
		return p1.get_priority() < p2.get_priority();
	}
}

bool compare_deadline(Process p1, Process p2){
	if(p1.get_deadline() == p2.get_deadline()){
		if(p1.get_arrival() == p2.get_arrival()){
			return p1.get_pid() > p2.get_pid();
		}else{
			return p1.get_arrival() > p2.get_arrival();
		}
	}else{
		return p1.get_deadline() > p2.get_deadline();
	}
}

bool compare_arrival(Process p1, Process p2){
	if(p1.get_arrival() == p2.get_arrival()){
		if(p1.get_deadline() == p2.get_deadline()){
			return p1.get_pid() > p2.get_pid();
		}else{
			return p1.get_deadline() > p2.get_deadline();
		}
	}else{
		return p1.get_arrival() > p2.get_arrival();
	}
}
