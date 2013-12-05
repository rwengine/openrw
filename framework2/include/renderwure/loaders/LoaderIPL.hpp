#ifndef _LOADERIPL_HPP_
#define _LOADERIPL_HPP_
#include <renderwure/data/InstanceData.hpp>

#include <iostream>
#include <vector>
#include <memory>

/**
	\class LoaderIPL
	\brief Loads all data from a IPL file into memory
*/
class LoaderIPL
{
public:
	
	/**
	 * \class Zone
	 *  A Zone entry
	 */
	class Zone 
	{
	public:
		/**
		 * The name of the Zone (see .gxt)
		 */
		std::string name;
		
		int type;
		
		/**
		 * Bottom left of the Zone
		 */
		glm::vec3 min;
	
		/** 
		 * Top Right of the zone
		 */
		glm::vec3 max;
		
		/**
		 * Island number
		 */
		int island;
		
		/**
		 * Text of the zone?
		 */
		std::string Text;
	};
	
	/// Load the IPL data into memory
	bool load(const std::string& filename);

	/// The list of instances from the IPL file
	std::vector<std::shared_ptr<InstanceData>> m_instances;
	
	/// List of Zones
	std::vector<Zone> zones;
};

#endif // LoaderIPL_h__
