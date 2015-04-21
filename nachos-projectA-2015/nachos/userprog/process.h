#ifdef CHANGED
#ifndef PROCESS_H
#define PROCESS_H

#include "list.h"
#include "thread.h"
#include <map>
//TODO: we should probably have some overarching comment at least to give some context
class Thread;
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

		static void SetZombie(int arg);
		void SetReturnValue(int val);
		int GetReturnValue();

		void SetFinished(bool f);
		bool GetFinished();

	private:
		int id;
		Process * parent;
		List * children;
		Thread * thread;
		static std::map<int, Process*>* procmap;
		int returnVal;
		bool finished;

};

#endif
#endif
