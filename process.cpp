#include "process.h"

Process::Process(int pid, int burst, int arrival, int priority, int deadline, int io){
	this->pid = pid;
	this->burst = burst;
	this->arrival = arrival;
	this->priority = priority;
	this->deadline = deadline;
	this->io = io;

}

void Process::print_rts() const{
	cout << "Pid: " << this->pid << endl;
	cout << "Burst: " << this->burst << endl;
	cout << "Arrival: " << this->arrival << endl;	
	cout << "Deadline: " << this->deadline << endl;
	cout << "\n";
}

void Process::print_mfqs() const{
	cout << "Pid: " << this->pid << endl;
	cout << "Burst: " << this->burst << endl;
	cout << "Arrival: " << this->arrival << endl;	
	cout << "priority: " << this->priority << endl;
	cout << "\n";
}

void Process::print_whs() const{
	cout << "Pid: " << this->pid << endl;
	cout << "Burst: " << this->burst << endl;
	cout << "Arrival: " << this->arrival << endl;	
	cout << "priority: " << this->priority << endl;
	cout << "\n";
}
