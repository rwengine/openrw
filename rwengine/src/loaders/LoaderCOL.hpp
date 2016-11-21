#ifndef _LOADERCOL_H_
#define _LOADERCOL_H_
#include <data/CollisionModel.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

/**
 * @class LoaderCOL
 *  Loads collision data from COL files
 */
class LoaderCOL {
public:
    /// Load the COL data into memory
    bool load(const std::string& file);

    /// Load the COL data from a file already in memory
    bool load(char* data, const size_t size);

    std::vector<std::unique_ptr<CollisionModel>> instances;
};

#endif
