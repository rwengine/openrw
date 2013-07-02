#ifndef LoaderIPL_h__
#define LoaderIPL_h__

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
	/// Load the IPL data into memory
	bool load(const std::string& filename);

	/// The list of instances from the IPL file
	std::vector<LoaderIPLInstance> m_instances;
	
	/// The centroid of the instances
	glm::vec3 centroid;
};

#endif // LoaderIPL_h__
