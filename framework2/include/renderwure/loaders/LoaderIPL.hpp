#ifndef _LOADERIPL_HPP_
#define _LOADERIPL_HPP_

#include <iostream>
#include <vector>
#include <glm/glm.hpp>

/**
  \class LoaderIPLInstance
	\brief One INST entry's data from a IPL file
*/
class LoaderIPLInstance
{
public:
	int id; ///< ID of the asset in the main IMG archive
	std::string model; ///< Name of the model for this instance, as seen in the IMG archive
	float posX, posY, posZ; ///< 3D Position of the instance
	float scaleX, scaleY, scaleZ; ///< Scale of the instance
	float rotX, rotY, rotZ, rotW; ///< Rotation of the instance, in a Quaternion
};

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
	std::vector<LoaderIPLInstance> m_instances;
	
	/// The centroid of the instances
	glm::vec3 centroid;
	
	/// List of Zones
	std::vector<Zone> zones;
};

#endif // LoaderIPL_h__
