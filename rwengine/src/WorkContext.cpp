#include <WorkContext.hpp>
#include <loaders/DataLoader.hpp>

void LoadWorker::start()
{
	while( _running ) {
		_context->workNext();
		std::this_thread::yield();
	}
}

void WorkContext::workNext()
{
	WorkJob* j = nullptr;
	{
		std::lock_guard<std::mutex> guard( _inMutex );
		if( _workQueue.empty() ) return;
		j = _workQueue.front(); _workQueue.pop();
	}

	j->work();

	{
		std::lock_guard<std::mutex> guard( _outMutex );
		_completeQueue.push( j );
	}
}

void WorkContext::update()
{
	std::lock_guard<std::mutex> guard( _outMutex );
	while( ! _completeQueue.empty() ) {
		WorkJob* j = _completeQueue.front(); _completeQueue.pop();
		j->complete();
		delete j;
	}
}

