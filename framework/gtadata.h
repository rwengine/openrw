#ifndef _GTADATA_H_
#define _GTADATA_H_
#include "gtfwpre.h"
#include <string>

/**
 * Handles loading and management of the Game's DAT
 */
class GTAData
{
	std::string datpath;
	std::string splash;
public:
	
	GTAData(const std::string& path);
	
	/**
	 * Returns the current platform
	 */
	std::string getPlatformString()
	{
		return "PC";
	}
	
	/**
	 * Loads the data contained in the given file
	 */
	void loadIDE(const std::string& name);
	
	/**
	 * Handles the parsing of a COL file.
	 */
	void loadCOL(const size_t zone, const std::string& name);
	
	void loadIPL(const std::string& name);
	
	void load();
};

#endif