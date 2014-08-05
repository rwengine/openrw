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

	_inMutex.lock();
	if( ! _workQueue.empty() ) {
		j = _workQueue.front();
		_workQueue.pop();
	}
	_inMutex.unlock();

	if( j == nullptr ) return;

	j->work();

	_outMutex.lock();
	_completeQueue.push(j);
	_outMutex.unlock();
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

