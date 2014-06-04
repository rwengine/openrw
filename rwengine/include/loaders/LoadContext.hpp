#pragma once
#ifndef _LOADCONTEXT_HPP_
#define _LOADCONTEXT_HPP_

#include <queue>
#include <thread>
#include <functional>
#include <fstream>

class DataLoader;
class LoadContext;

struct LoadRequest
{
	DataLoader* loader;
	const char* data;
	size_t size;
	std::string filename;
};

class LoadWorker
{
	LoadContext* _context;

public:

	bool _started;
	bool _running;
	std::thread _thread;
	void start();

	LoadWorker( LoadContext* context )
		: _context( context ), _started(false), _running(true),
	_thread( std::bind(&LoadWorker::start, this) ) { }

	~LoadWorker( )
	{
		_started = true;
		_running = false;
		_thread.join();
	}
};

class LoadContext
{
	std::queue<LoadRequest> _loadQueue;
	std::queue<LoadRequest> _createQueue;
	unsigned int _loaded;
	unsigned int _loading;

	LoadWorker _worker;

public:

	LoadContext()
		: _loaded(0), _loading(0), _worker(this) { }

	void add(DataLoader* loader, const char* data, size_t size)
	{
		_loadQueue.push({loader, data, size, ""});
	}

	void add(DataLoader* loader, const std::string& filename)
	{
		_loadQueue.push({loader, nullptr, 0, filename});
	}

	void loadNext();

	const std::queue<LoadRequest> getLoadQueue() const { return _loadQueue; }

	unsigned int getLoaded() const { return _createQueue.size(); }

	unsigned int getIncomplete() const { return _loadQueue.size() + _loading + _createQueue.size(); }

	unsigned int getComplete() const { return _loaded; }

	unsigned int getTotal() const { return getIncomplete() + getComplete(); }

	void flushCreation();

	void wait();

};

#endif
