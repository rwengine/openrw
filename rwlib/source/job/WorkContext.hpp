#pragma once
#ifndef _LOADCONTEXT_HPP_
#define _LOADCONTEXT_HPP_

#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <functional>
#include <fstream>
#include <iostream>

class WorkContext;

class LoadWorker
{
	WorkContext* _context;

public:

	std::atomic<bool> _running;
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

/**
 * @brief Interface for background work
 */
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

// TODO: refactor everything to remove this.
class GameWorld;

/**
 * @brief A worker queue that runs work in the background.
 *
 * Work is added with queueJob, once it completes the job is added
 * to the _completeQueue to be finalised on the "main" thread.
 */
class WorkContext
{
	std::unique_ptr<LoadWorker> _worker;

	std::queue<WorkJob*> _workQueue;
	std::queue<WorkJob*> _completeQueue;

	std::mutex _inMutex;
	std::mutex _outMutex;

public:

	WorkContext()
		: _worker(new LoadWorker(this)) { }

	void queueJob( WorkJob* job )
	{
		std::lock_guard<std::mutex> guard( _inMutex );
		_workQueue.push( job );
	}

	void stop()
	{
		// Stop serving the queue.
		_worker.reset(nullptr);
	}

	// Called by the worker thread - don't touch
	void workNext();

	const std::queue<WorkJob*> getWorkQueue() const { return _workQueue; }
	const std::queue<WorkJob*> getCompleteQueue() const { return _completeQueue; }

	bool isEmpty() {
		std::lock_guard<std::mutex> guardIn( _inMutex );
		std::lock_guard<std::mutex> guardOu( _outMutex );

		return (getWorkQueue().size() + getCompleteQueue().size()) == 0;
	}

	void update();
};

#endif
