#ifndef RWENGINE_LOADERCOL_HPP
#define RWENGINE_LOADERCOL_HPP
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

    std::vector<std::unique_ptr<CollisionModel>> collisions;
};

#endif
