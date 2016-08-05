#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <functional>
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

/**
 * @brief A worker queue that runs work in the background.
 *
 * Work is added with queueJob, once it completes the job is added
 * to the _completeQueue to be finalised on the "main" thread.
 */
class WorkContext
{
	std::mutex _inMutex;
	std::mutex _outMutex;

	std::queue<WorkJob*> _workQueue;
	std::queue<WorkJob*> _completeQueue;

	// Construct the worker last, so that it may use the queues
	// immediately after initialization.
	std::unique_ptr<LoadWorker> _worker;

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

	bool isEmpty() {
		std::lock_guard<std::mutex> guardIn( _inMutex );
		std::lock_guard<std::mutex> guardOut( _outMutex );

		return (_workQueue.size() + _completeQueue.size()) == 0;
	}

	void update();
};
