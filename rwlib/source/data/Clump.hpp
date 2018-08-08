#ifndef _LIBRW_CLUMP_HPP_
#define _LIBRW_CLUMP_HPP_
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <GL/glew.h>
#include <gl/DrawBuffer.hpp>
#include <gl/GeometryBuffer.hpp>
#include <gl/TextureData.hpp>
#include <loaders/RWBinaryStream.hpp>

#include <rw/forward.hpp>

/**
 * ModelFrame stores transformation hierarchy
 */
class ModelFrame {
    unsigned int index;
    glm::mat3 defaultRotation;
    glm::vec3 defaultTranslation;
    glm::mat4 matrix{1.0f};
    glm::mat4 worldtransform_{1.0f};
    ModelFrame* parent_;
    std::string name;
    std::vector<ModelFramePtr> children_;

public:
    ModelFrame(unsigned int index = 0, glm::mat3 dR = glm::mat3{1.0f},
               glm::vec3 dT = glm::vec3());

    void reset();

    void setTransform(const glm::mat4& m) {
        matrix = m;
        updateHierarchyTransform();
    }

    const glm::mat4& getTransform() const {
        return matrix;
    }

    void setName(const std::string& fname) {
        name = fname;
    }

    unsigned int getIndex() const {
        return index;
    }

    glm::vec3 getDefaultTranslation() const {
        return defaultTranslation;
    }

    glm::mat3 getDefaultRotation() const {
        return defaultRotation;
    }

    void setTranslation(const glm::vec3& t) {
        matrix[3] = glm::vec4(t, matrix[3][3]);
        updateHierarchyTransform();
    }

    void setRotation(const glm::mat3& r) {
        for (unsigned int i = 0; i < 3; i++) {
            matrix[i] = glm::vec4(r[i], matrix[i][3]);
        }
        updateHierarchyTransform();
    }

    /**
     * Updates the cached matrix
     */
    void updateHierarchyTransform();

    /**
     * @return the cached world transformation for this Frame
     */
    const glm::mat4& getWorldTransform() const {
        return worldtransform_;
    }

    ModelFrame* getParent() const {
        return parent_;
    }

    void addChild(const ModelFramePtr& child);

    const std::vector<ModelFramePtr>& getChildren() const {
        return children_;
    }

    const std::string& getName() const {
        return name;
    }

    ModelFrame* findDescendant(const std::string& name) const;

    ModelFramePtr cloneHierarchy() const;
};

/**
 * Subgeometry
 */

struct SubGeometry {
    GLuint start = 0;
    size_t material = 0;
    std::vector<uint32_t> indices;
    size_t numIndices = 0;
};

struct GeometryVertex {
    glm::vec3 position{}; /* 0 */
    glm::vec3 normal{};   /* 24 */
    glm::vec2 texcoord{}; /* 48 */
    glm::u8vec4 colour{}; /* 64 */

    /** @see GeometryBuffer */
    static const AttributeList vertex_attributes() {
        return {{ATRS_Position, 3, sizeof(GeometryVertex), 0ul},
                {ATRS_Normal, 3, sizeof(GeometryVertex), sizeof(float) * 3},
                {ATRS_TexCoord, 2, sizeof(GeometryVertex), sizeof(float) * 6},
                {ATRS_Colour, 4, sizeof(GeometryVertex), sizeof(float) * 8,
                 GL_UNSIGNED_BYTE}};
    }

    GeometryVertex(glm::vec3 _position, glm::vec3 _normal, glm::vec2 _texcoord, glm::u8vec4 _colour)
        : position(_position)
        , normal(_normal)
        , texcoord(_texcoord)
        , colour(_colour) {
    }

    GeometryVertex() = default;
};

/**
 * Geometry
 */
struct Geometry {
    enum FaceType { Triangles = 0, TriangleStrip = 1 };

    struct Texture {
        std::string name;
        std::string alphaName;
        TextureData::Handle texture;

        template<class String1, class String2>
        Texture(String1&& _name, String2&& _alphaName, const TextureData::Handle &_texture)
            : name(std::forward<String1>(_name))
            , alphaName(std::forward<String2>(_alphaName))
            , texture(_texture) {
        }
    };

    enum { MTF_PrimaryColour = 1 << 0, MTF_SecondaryColour = 1 << 1 };

    struct Material {
        std::vector<Texture> textures;
        glm::u8vec4 colour{};

        uint8_t flags;

        float diffuseIntensity;
        float ambientIntensity;
    };

    DrawBuffer dbuff;
    GeometryBuffer gbuff;

    GLuint EBO;

    RW::BSGeometryBounds geometryBounds;

    uint32_t clumpNum;

    FaceType facetype;

    uint32_t flags;

    std::vector<Material> materials;
    std::vector<SubGeometry> subgeom;

    Geometry();
    ~Geometry();
};

/**
 * @brief The Atomic struct
 */
class Atomic {
    ModelFramePtr frame_;
    GeometryPtr geometry_;
    uint32_t flags_;

public:
    enum {
        // If this is set, the atomic will be rendered
        ATOMIC_RENDER = 0x04
    };

    void setFrame(const ModelFramePtr& frame) {
        frame_ = frame;
    }

    const ModelFramePtr& getFrame() const {
        return frame_;
    }

    void setGeometry(const GeometryPtr& geom) {
        geometry_ = geom;
    }

    const GeometryPtr& getGeometry() const {
        return geometry_;
    }

    void setFlags(uint32_t flags) {
        flags_ = flags;
    }

    uint32_t getFlags() const {
        return flags_;
    }

    void setFlag(uint32_t flag, bool set) {
        if (set) {
            flags_ |= flag;
        }
        else {
            flags_ &= ~flag;
        }
    }

    AtomicPtr clone() const;
};

/**
 * A clump is a collection of Frames and Atomics
 */
class Clump {
public:
    /**
     * @brief findFrame Locates frame with name anywhere in the hierarchy
     * @param name
     * @return
     */
    ModelFrame* findFrame(const std::string& name) const;

    ~Clump();

    void recalculateMetrics();

    float getBoundingRadius() const {
        return boundingRadius;
    }

    void addAtomic(const AtomicPtr& atomic) {
        atomics_.push_back(atomic);
    }

    const AtomicList& getAtomics() const {
        return atomics_;
    }

    void setFrame(const ModelFramePtr& root) {
        rootframe_ = root;
    }

    const ModelFramePtr& getFrame() const {
        return rootframe_;
    }

    /**
     * @return A Copy of the frames and atomics in this clump
     */
    Clump* clone() const;

private:
    float boundingRadius;
    AtomicList atomics_;
    ModelFramePtr rootframe_;
};

#endif
