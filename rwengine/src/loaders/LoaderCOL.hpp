#ifndef _RWENGINE_LOADERCOL_HPP_
#define _RWENGINE_LOADERCOL_HPP_

#include <memory>
#include <string>
#include <vector>

struct CollisionModel;

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
