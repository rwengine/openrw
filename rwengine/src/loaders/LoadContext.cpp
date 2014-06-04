#include <loaders/LoadContext.hpp>
#include <loaders/DataLoader.hpp>

void LoadWorker::start()
{
	while( !_started ) {
		std::this_thread::yield();
	}
	while( _running ) {
		_context->loadNext();
		std::this_thread::yield();
	}
}


void LoadContext::loadNext()
{
	// MUTEX
	if( _loadQueue.size() == 0 ) {
		// UMUTEX
		return;
	}
	_loading++;
	auto tl = _loadQueue.front();
	_loadQueue.pop();
	// UMUTEX

	if( tl.data == nullptr ) {
		// TODO: instigate opening of the data bytes.
	}

	bool result = tl.loader->load(tl.data, tl.size);

	delete tl.data;

	// MUTEX

	if( result ) {
		_createQueue.push(tl);
	}
	else {
		// Create is rip
		_loaded++;
	}
	_loading--;

	// UMUTEX
}

void LoadContext::flushCreation()
{
	// MUTEX

	while( ! _createQueue.empty() ) {
		auto tl = _createQueue.front();
		_createQueue.pop();

		tl.loader->create();
		_loaded++;
	}

	// UMUTEX
}

void LoadContext::wait()
{
	while( _loadQueue.size() > 0 || _loading > 0 ) std::this_thread::yield();
}

