#pragma once
#ifndef _LOADCONTEXT_HPP_
#define _LOADCONTEXT_HPP_

#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <fstream>

class WorkContext;

class LoadWorker
{
	WorkContext* _context;

public:

	bool _running;
	std::thread _thread;
	void start();

	LoadWorker( WorkContext* context )
		: _context( context ), _running(true),
	_thread( std::bind(&LoadWorker::start, this) ) { }

	~LoadWorker( )
	{
		_running = false;
		_thread.join();
	}
};

class WorkJob
{
	WorkContext* _context;

public:

	WorkJob(WorkContext* context)
		: _context(context) {}

	virtual ~WorkJob() {}

	/**
	 * @brief getContext
	 * @return The loading context for this Loader
	 */
	WorkContext* getContext() const { return _context; }

	virtual void work() = 0;
	virtual void complete() {}
};

class WorkContext
{
	std::queue<WorkJob*> _workQueue;
	std::queue<WorkJob*> _completeQueue;

	LoadWorker _worker;

	std::mutex _inMutex;
	std::mutex _outMutex;

public:

	WorkContext()
		: _worker(this) { }

	void queueJob( WorkJob* job )
	{
		std::lock_guard<std::mutex> guard(_inMutex);
		_workQueue.push( job );
	}

	// Called by the worker thread - don't touch;
	void workNext();

	const std::queue<WorkJob*> getWorkQueue() const { return _workQueue; }
	const std::queue<WorkJob*> getCompleteQueue() const { return _completeQueue; }

	void update();
};

#endif
