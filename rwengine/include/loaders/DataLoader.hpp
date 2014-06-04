#pragma once
#ifndef _DATALOADER_HPP_
#define _DATALOADER_HPP_

#include <iostream>

class WorkContext;

class DataLoader
{
	WorkContext* _context;

public:

	DataLoader(WorkContext* context)
		: _context(context) {}

	virtual ~DataLoader() {}

	/**
	 * @brief getContext
	 * @return The loading context for this Loader
	 */
	WorkContext* getContext() const { return _context; }

	/**
	 * @brief load the data contained in a set of bytes
	 * @param data The bytes from which to load data.
	 * @param size The number of bytes.
	 * @return true if the data was valid and loaded, false otherwise.
	 */
	virtual bool load( const char* data, size_t size ) = 0;

	/**
	 * @brief create perform any after-load activitiy that is required.
	 */
	virtual void create() = 0;
};


#endif
