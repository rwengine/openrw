#pragma once

#include <WorkContext.hpp>

/**
 * Generic base class for loader implementations
 */
template<class T> class Loader
{
public:
	/**
	 * Type of the resource produced by the loader
	 */
	typedef T ResultType;
	
	/**
	 * Method that should be used to export the loaded data
	 */
	ResultType get();
};
