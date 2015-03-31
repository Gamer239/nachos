#ifndef PROCESS_H
#define PROCESS_H

#include "list.h"
#include "thread.h"
#include <map>

class Process {
	
	public:
		Process(Thread *thread, int id);
		~Process();

		void SetParent(Process* parent);
		Process * GetParent();

		void AddChild(Process* child);
		List * GetChildren();

		static std::map<int, Process*>* GetProcMap();

		Thread * GetThread();

	private: 
		int id;
		Process * parent;
		List * children;
		Thread * thread;
		static std::map<int, Process*>* procmap;

};

#endif
