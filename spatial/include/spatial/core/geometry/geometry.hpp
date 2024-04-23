#pragma once

#include "spatial/common.hpp"
#include "spatial/core/geometry/geometry_properties.hpp"
#include "spatial/core/util/cursor.hpp"
#include "spatial/core/util/misaligned_ptr.hpp"
#include "spatial/core/geometry/geometry_type.hpp"
#include "spatial/core/geometry/vertex.hpp"

namespace spatial {

namespace core {

class Geometry;

//------------------------------------------------------------------------------
// Geometry
//------------------------------------------------------------------------------

// Forward declare all accessors
struct AnyGeometry;
struct SinglePartGeometry;
struct MultiPartGeometry;
struct CollectionGeometry;
struct Point;
struct LineString;
struct Polygon;
struct MultiPoint;
struct MultiLineString;
struct MultiPolygon;
struct GeometryCollection;

class Geometry {
    friend struct SinglePartGeometry;
    friend struct MultiPartGeometry;
    friend struct CollectionGeometry;
private:
    GeometryType type;
    GeometryProperties properties;
    bool is_readonly;
    uint32_t data_count;
    data_ptr_t data_ptr;

    Geometry(GeometryType type, GeometryProperties props, bool is_readonly, data_ptr_t data, uint32_t count)
        : type(type), properties(props), is_readonly(is_readonly), data_count(count), data_ptr(data) {}

public:
    // By default, create a read-only empty point
    Geometry() : type(GeometryType::POINT), properties(false, false), is_readonly(true), data_count(0), data_ptr(nullptr) {}

    Geometry(GeometryType type, bool has_z, bool has_m)
            : type(type), properties(has_z, has_m), is_readonly(true), data_count(0), data_ptr(nullptr) {}

    // Copy Constructor
    Geometry(const Geometry &other)
            : type(other.type), properties(other.properties), is_readonly(true), data_count(other.data_count),
              data_ptr(other.data_ptr) { }

    // Copy Assignment
    Geometry &operator=(const Geometry &other) {
        type = other.type;
        properties = other.properties;
        is_readonly = true;
        data_count = other.data_count;
        data_ptr = other.data_ptr;
        return *this;
    }

    // Move Constructor
    Geometry(Geometry &&other) noexcept
    : type(other.type), properties(other.properties), is_readonly(other.is_readonly), data_count(other.data_count),
    data_ptr(other.data_ptr) {
        if (!other.is_readonly) {
            // Take ownership of the data, and make the other object read-only
            other.is_readonly = true;
        }
    }

    // Move Assignment
    Geometry &operator=(Geometry &&other) noexcept {
        type = other.type;
        properties = other.properties;
        is_readonly = other.is_readonly;
        data_count = other.data_count;
        data_ptr = other.data_ptr;
        if (!other.is_readonly) {
            // Take ownership of the data, and make the other object read-only
            other.is_readonly = true;
        }
        return *this;
    }

public:
    GeometryType GetType() const { return type; }
    GeometryProperties &GetProperties() { return properties; }
    const GeometryProperties &GetProperties() const { return properties; }
    const_data_ptr_t GetData() const { return data_ptr; }
    data_ptr_t GetData() { return data_ptr; }
    bool IsReadOnly() const { return is_readonly; }
    uint32_t Count() const { return data_count; }

    bool IsCollection() const { return GeometryTypes::IsCollection(type); }
    bool IsMultiPart() const { return GeometryTypes::IsMultiPart(type); }
    bool IsSinglePart() const { return GeometryTypes::IsSinglePart(type); }

public:
    // Used for tag dispatching
    struct Tags {
        // Base types
        struct AnyGeometry {
            using ACCESS = ::spatial::core::AnyGeometry;
        };
        struct SinglePartGeometry : public AnyGeometry {
            using ACCESS = ::spatial::core::SinglePartGeometry;
        };
        struct MultiPartGeometry : public AnyGeometry {
            using ACCESS = ::spatial::core::MultiPartGeometry;
        };
        struct CollectionGeometry : public MultiPartGeometry {
            using ACCESS = ::spatial::core::CollectionGeometry;
        };
        // Concrete types
        struct Point : public SinglePartGeometry {
            using ACCESS = ::spatial::core::Point;
        };
        struct LineString : public SinglePartGeometry {
            using ACCESS = ::spatial::core::LineString;
        };
        struct Polygon : public MultiPartGeometry {
            using ACCESS = ::spatial::core::Polygon;
        };
        struct MultiPoint : public CollectionGeometry {
            using ACCESS = ::spatial::core::MultiPoint;
        };
        struct MultiLineString : public CollectionGeometry {
            using ACCESS = ::spatial::core::MultiLineString;
        };
        struct MultiPolygon : public CollectionGeometry {
            using ACCESS = ::spatial::core::MultiPolygon;
        };
        struct GeometryCollection : public CollectionGeometry {
            using ACCESS = ::spatial::core::GeometryCollection;
        };
    };

    template<class T, class ...ARGS>
    static auto Visit(Geometry& geom, ARGS&&... args) -> decltype(T::Case(std::declval<Tags::Point>(), std::declval<Geometry&>(), std::declval<ARGS>()...)) {
        switch(geom.type) {
            case GeometryType::POINT: return T::Case(Tags::Point{}, geom, std::forward<ARGS>(args)...);
            case GeometryType::LINESTRING: return T::Case(Tags::LineString{}, geom, std::forward<ARGS>(args)...);
            case GeometryType::POLYGON: return T::Case(Tags::Polygon{}, geom, std::forward<ARGS>(args)...);
            case GeometryType::MULTIPOINT: return T::Case(Tags::MultiPoint{}, geom, std::forward<ARGS>(args)...);
            case GeometryType::MULTILINESTRING: return T::Case(Tags::MultiLineString{}, geom, std::forward<ARGS>(args)...);
            case GeometryType::MULTIPOLYGON: return T::Case(Tags::MultiPolygon{}, geom, std::forward<ARGS>(args)...);
            case GeometryType::GEOMETRYCOLLECTION: return T::Case(Tags::GeometryCollection{}, geom, std::forward<ARGS>(args)...);
            default: throw NotImplementedException("Geometry::Match");
        }
    }

    template<class T, class ...ARGS>
    static auto Visit(const Geometry &geom, ARGS&&... args) -> decltype(T::Case(std::declval<Tags::Point>(), std::declval<Geometry&>(), std::declval<ARGS>()...))  {
        switch(geom.type) {
            case GeometryType::POINT: return T::Case(Tags::Point{}, geom, std::forward<ARGS>(args)...);
            case GeometryType::LINESTRING: return T::Case(Tags::LineString{}, geom, std::forward<ARGS>(args)...);
            case GeometryType::POLYGON: return T::Case(Tags::Polygon{}, geom, std::forward<ARGS>(args)...);
            case GeometryType::MULTIPOINT: return T::Case(Tags::MultiPoint{}, geom, std::forward<ARGS>(args)...);
            case GeometryType::MULTILINESTRING: return T::Case(Tags::MultiLineString{}, geom, std::forward<ARGS>(args)...);
            case GeometryType::MULTIPOLYGON: return T::Case(Tags::MultiPolygon{}, geom, std::forward<ARGS>(args)...);
            case GeometryType::GEOMETRYCOLLECTION: return T::Case(Tags::GeometryCollection{}, geom, std::forward<ARGS>(args)...);
            default: throw NotImplementedException("Geometry::Match");
        }
    }

    // TODO: Swap this to only have two create methods,
    // and use mutating methods for Reference/Copy
    static Geometry Create(ArenaAllocator &alloc, GeometryType type, uint32_t count, bool has_z, bool has_m);
    static Geometry CreateEmpty(GeometryType type, bool has_z, bool has_m);

    static geometry_t Serialize(const Geometry &geom, Vector &result);
    static Geometry Deserialize(ArenaAllocator &arena, const geometry_t &data);

    static bool IsEmpty(const Geometry &geom);
    static uint32_t GetDimension(const Geometry &geom, bool recurse);
    void SetVertexType(ArenaAllocator &alloc, bool has_z, bool has_m, double default_z = 0, double default_m = 0);

    // Iterate over all points in the geometry, recursing into collections
    template<class FUNC>
    static void ExtractPoints(const Geometry &geom, FUNC &&func);

    // Iterate over all lines in the geometry, recursing into collections
    template<class FUNC>
    static void ExtractLines(const Geometry &geom, FUNC &&func);

    // Iterate over all polygons in the geometry, recursing into collections
    template<class FUNC>
    static void ExtractPolygons(const Geometry &geom, FUNC &&func);
};

inline Geometry Geometry::Create(ArenaAllocator &alloc, GeometryType type, uint32_t count, bool has_z, bool has_m) {
    GeometryProperties props(has_z, has_m);
    auto single_part = GeometryTypes::IsSinglePart(type);
    auto elem_size = single_part ? props.VertexSize() : sizeof(Geometry);
    auto geom = Geometry(type, props, false, alloc.AllocateAligned(count * elem_size), count);
    return geom;
}

inline Geometry Geometry::CreateEmpty(GeometryType type, bool has_z, bool has_m) {
    GeometryProperties props(has_z, has_m);
    return Geometry(type, props, false, nullptr, 0);
}

//------------------------------------------------------------------------------
// Iterators
//------------------------------------------------------------------------------
class PartView {
private:
    Geometry* beg_ptr;
    Geometry* end_ptr;
public:
    PartView(Geometry *beg, Geometry *end) : beg_ptr(beg), end_ptr(end) {}
    Geometry* begin() { return beg_ptr; }
    Geometry* end() { return end_ptr; }
    Geometry& operator[](uint32_t index) { return beg_ptr[index]; }
};

class ConstPartView {
private:
    const Geometry* beg_ptr;
    const Geometry* end_ptr;
public:
    ConstPartView(const Geometry *beg, const Geometry *end) : beg_ptr(beg), end_ptr(end) {}
    const Geometry* begin() { return beg_ptr; }
    const Geometry* end() { return end_ptr; }
    const Geometry& operator[](uint32_t index) { return beg_ptr[index]; }
};

class VertexView {
private:
    data_ptr_t beg_ptr;
    data_ptr_t end_ptr;
    uint32_t vertex_size;
public:
    VertexView(data_ptr_t beg, data_ptr_t end, uint32_t vertex_size)
        : beg_ptr(beg), end_ptr(end), vertex_size(vertex_size) {}

    StridedPtr<VertexXY> begin() { return { beg_ptr, vertex_size }; }
    StridedPtr<VertexXY> end() { return { end_ptr, vertex_size }; }
    MisalignedRef<VertexXY> operator[](uint32_t index) {
        D_ASSERT(beg_ptr + index * vertex_size < end_ptr);
        return { beg_ptr + index * vertex_size };
    }
};

class ConstVertexView {
private:
    const_data_ptr_t beg_ptr;
    const_data_ptr_t end_ptr;
    uint32_t vertex_size;
public:
    ConstVertexView(const_data_ptr_t beg, const_data_ptr_t end, uint32_t vertex_size)
        : beg_ptr(beg), end_ptr(end), vertex_size(vertex_size) {}

    ConstStridedPtr<VertexXY> begin() const { return { beg_ptr, vertex_size }; }
    ConstStridedPtr<VertexXY> end() const { return { end_ptr, vertex_size }; }
    ConstMisalignedRef<VertexXY> operator[](uint32_t index) {
        D_ASSERT(beg_ptr + index * vertex_size < end_ptr);
        return { beg_ptr + index * vertex_size };
    }
};

template<class V>
class TypedVertexView {
    static_assert(V::IS_VERTEX, "V must be a vertex type");
    data_ptr_t beg_ptr;
    data_ptr_t end_ptr;
public:
    TypedVertexView(data_ptr_t beg, data_ptr_t end)
        : beg_ptr(beg), end_ptr(end) {}

    MisalignedPtr<V> begin() { return { beg_ptr }; }
    MisalignedPtr<V> end() { return { end_ptr }; }
    MisalignedRef<V> operator[](uint32_t index) {
        D_ASSERT(beg_ptr + index * sizeof(V) < end_ptr);
        return { beg_ptr + index * sizeof(V) };
    }
};

template<class V>
class ConstTypedVertexView {
    static_assert(V::IS_VERTEX, "V must be a vertex type");
    const_data_ptr_t beg_ptr;
    const_data_ptr_t end_ptr;
public:
    ConstTypedVertexView(const_data_ptr_t beg, const_data_ptr_t end)
        : beg_ptr(beg), end_ptr(end) {}

    ConstMisalignedPtr<V> begin() { return { beg_ptr }; }
    ConstMisalignedPtr<V> end() { return { end_ptr }; }
    ConstMisalignedRef<V> operator[](uint32_t index) {
        D_ASSERT(beg_ptr + index * sizeof(V) < end_ptr);
        return { beg_ptr + index * sizeof(V) };
    }
};

//------------------------------------------------------------------------------
// Accessors
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// AnyGeometry
//------------------------------------------------------------------------------
struct AnyGeometry {};

//------------------------------------------------------------------------------
// SinglePartGeometry
//------------------------------------------------------------------------------
struct SinglePartGeometry {

    // Turn this geometry into a read-only reference to raw data
    static void ReferenceData(Geometry &geom, const_data_ptr_t data, uint32_t count, bool has_z, bool has_m) {
        geom.data_count = count;
        geom.data_ptr = const_cast<data_ptr_t>(data);
        geom.is_readonly = true;
        geom.properties.SetZ(has_z);
        geom.properties.SetM(has_m);
    }

    // Turn this geometry into a read-only reference to another geometry, starting at the specified index
    static void ReferenceData(Geometry &geom, const Geometry &other, uint32_t offset, uint32_t count) {
        D_ASSERT(GeometryTypes::IsSinglePart(other.GetType()));
        D_ASSERT(offset + count <= other.data_count);
        auto vertex_size = other.properties.VertexSize();
        auto has_z = other.properties.HasZ();
        auto has_m = other.properties.HasM();
        ReferenceData(geom, other.data_ptr + offset * vertex_size, count, has_z, has_m);
    }

    static void ReferenceData(Geometry &geom, const_data_ptr_t data, uint32_t count) {
        ReferenceData(geom, data, count, geom.properties.HasZ(), geom.properties.HasM());
    }

    // Turn this geometry into a owning copy of raw data
    static void CopyData(Geometry &geom, ArenaAllocator &alloc, const_data_ptr_t data, uint32_t count, bool has_z, bool has_m) {
        auto old_vertex_size = geom.properties.VertexSize();
        geom.properties.SetZ(has_z);
        geom.properties.SetM(has_m);
        auto new_vertex_size = geom.properties.VertexSize();
        if(geom.is_readonly) {
            geom.data_ptr = alloc.AllocateAligned(count * new_vertex_size);
        } else if(geom.data_count != count) {
            geom.data_ptr = alloc.ReallocateAligned(geom.data_ptr, geom.data_count * old_vertex_size, count * new_vertex_size);
        }
        memcpy(geom.data_ptr, data, count * new_vertex_size);
        geom.data_count = count;
        geom.is_readonly = false;
    }

    // Turn this geometry into a owning copy of another geometry, starting at the specified index
    static void CopyData(Geometry &geom, ArenaAllocator &alloc, const Geometry &other, uint32_t offset, uint32_t count) {
        D_ASSERT(GeometryTypes::IsSinglePart(other.GetType()));
        D_ASSERT(offset + count <= other.data_count);
        auto vertex_size = geom.properties.VertexSize();
        auto has_z = other.properties.HasZ();
        auto has_m = other.properties.HasM();
        CopyData(geom, alloc, other.data_ptr + offset * vertex_size, count, has_z, has_m);
    }

    static void CopyData(Geometry &geom, ArenaAllocator &alloc, const_data_ptr_t data, uint32_t count) {
        CopyData(geom, alloc, data, count, geom.properties.HasZ(), geom.properties.HasM());
    }

    // Resize the geometry, truncating or extending with zeroed vertices as needed
    static void Resize(Geometry &geom, ArenaAllocator &alloc, uint32_t new_count);

    // Append the data from another geometry
    static void Append(Geometry &geom, ArenaAllocator &alloc, const Geometry &other);

    // Append the data from multiple other geometries
    static void Append(Geometry &geom, ArenaAllocator &alloc, const Geometry *others, uint32_t others_count);

    // Force the geometry to have a specific vertex type, resizing or shrinking the data as needed
    static void SetVertexType(Geometry &geom, ArenaAllocator &alloc, bool has_z, bool has_m, double default_z = 0, double default_m = 0);

    // If this geometry is read-only, make it mutable by copying the data
    static void MakeMutable(Geometry &geom, ArenaAllocator &alloc);

    // Print this geometry as a string, starting at the specified index and printing the specified number of vertices
    // (useful for debugging)
    static string ToString(const Geometry &geom, uint32_t start = 0, uint32_t count = 0);

    // Check if the geometry is closed (first and last vertex are the same)
    // A geometry with 1 vertex is considered closed, 0 vertices are considered open
    static bool IsClosed(const Geometry &geom);
    static bool IsEmpty(const Geometry &geom);

    // Return the planar length of the geometry
    static double Length(const Geometry &geom);

    static VertexXY GetVertex(const Geometry &geom, uint32_t index);
    static void SetVertex(Geometry &geom, uint32_t index, const VertexXY &vertex);

    template<class V>
    static V GetVertex(const Geometry &geom, uint32_t index);

    template<class V>
    static void SetVertex(Geometry &geom, uint32_t index, const V &vertex);

    template<class V>
    static MisalignedRef<V> Vertex(Geometry &geom, uint32_t index);
    static MisalignedRef<VertexXY> Vertex(Geometry &geom, uint32_t index);

    template<class V>
    static ConstMisalignedRef<V> Vertex(const Geometry &geom, uint32_t index);
    static ConstMisalignedRef<VertexXY> Vertex(const Geometry &geom, uint32_t index);

    static uint32_t VertexCount(const Geometry &geom);
    static uint32_t VertexSize(const Geometry &geom);
    static uint32_t ByteSize(const Geometry &geom);

    static VertexView Vertices(Geometry &geom) {
        D_ASSERT(GeometryTypes::IsSinglePart(geom.GetType()));
        auto count = VertexCount(geom);
        auto size = VertexSize(geom);
        return { geom.GetData(), geom.GetData() + count * size, size };
    }

    static ConstVertexView Vertices(const Geometry &geom) {
        D_ASSERT(GeometryTypes::IsSinglePart(geom.GetType()));
        auto count = VertexCount(geom);
        auto size = VertexSize(geom);
        return { geom.GetData(), geom.GetData() + count * size, size };
    }
};

inline VertexXY SinglePartGeometry::GetVertex(const Geometry &geom, uint32_t index) {
    D_ASSERT(GeometryTypes::IsSinglePart(geom.GetType()));
    D_ASSERT(index < geom.data_count);
    return Load<VertexXY>(geom.GetData() + index * geom.GetProperties().VertexSize());
}

inline void SinglePartGeometry::SetVertex(Geometry &geom, uint32_t index, const VertexXY &vertex) {
    D_ASSERT(GeometryTypes::IsSinglePart(geom.GetType()));
    D_ASSERT(index < geom.data_count);
    Store(vertex, geom.GetData() + index * geom.GetProperties().VertexSize());
}

template<class V>
inline V SinglePartGeometry::GetVertex(const Geometry &geom, uint32_t index) {
    D_ASSERT(GeometryTypes::IsSinglePart(geom.GetType()));
    D_ASSERT(V::HAS_Z == geom.GetProperties().HasZ());
    D_ASSERT(V::HAS_M == geom.GetProperties().HasM());
    D_ASSERT(index < geom.data_count);
    return Load<V>(geom.GetData() + index * sizeof(V));
}

template<class V>
inline void SinglePartGeometry::SetVertex(Geometry &geom, uint32_t index, const V &vertex) {
    D_ASSERT(GeometryTypes::IsSinglePart(geom.GetType()));
    D_ASSERT(V::HAS_Z == geom.GetProperties().HasZ());
    D_ASSERT(V::HAS_M == geom.GetProperties().HasM());
    D_ASSERT(index < geom.data_count);
    Store(vertex, geom.GetData() + index * sizeof(V));
}

template<class V>
inline MisalignedRef<V> SinglePartGeometry::Vertex(Geometry &geom, uint32_t index) {
    D_ASSERT(GeometryTypes::IsSinglePart(geom.GetType()));
    D_ASSERT(V::HAS_Z == geom.GetProperties().HasZ());
    D_ASSERT(V::HAS_M == geom.GetProperties().HasM());
    D_ASSERT(index < geom.data_count);
    return { geom.GetData() + index * sizeof(V) };
}

inline MisalignedRef<VertexXY> SinglePartGeometry::Vertex(Geometry &geom, uint32_t index) {
    D_ASSERT(GeometryTypes::IsSinglePart(geom.GetType()));
    D_ASSERT(index < geom.data_count);
    return { geom.GetData() + index * geom.GetProperties().VertexSize() };
}

template<class V>
inline ConstMisalignedRef<V> SinglePartGeometry::Vertex(const Geometry &geom, uint32_t index) {
    D_ASSERT(GeometryTypes::IsSinglePart(geom.GetType()));
    D_ASSERT(V::HAS_Z == geom.GetProperties().HasZ());
    D_ASSERT(V::HAS_M == geom.GetProperties().HasM());
    D_ASSERT(index < geom.data_count);
    return { geom.GetData() + index * sizeof(V) };
}

inline ConstMisalignedRef<VertexXY> SinglePartGeometry::Vertex(const Geometry &geom, uint32_t index) {
    D_ASSERT(GeometryTypes::IsSinglePart(geom.GetType()));
    D_ASSERT(index < geom.data_count);
    return { geom.GetData() + index * geom.GetProperties().VertexSize() };
}

inline uint32_t SinglePartGeometry::VertexCount(const Geometry &geom) {
    D_ASSERT(GeometryTypes::IsSinglePart(geom.GetType()));
    return geom.data_count;
}

inline uint32_t SinglePartGeometry::VertexSize(const Geometry &geom) {
    D_ASSERT(GeometryTypes::IsSinglePart(geom.GetType()));
    return geom.GetProperties().VertexSize();
}

inline uint32_t SinglePartGeometry::ByteSize(const Geometry &geom) {
    D_ASSERT(GeometryTypes::IsSinglePart(geom.GetType()));
    return geom.data_count * geom.GetProperties().VertexSize();
}

inline bool SinglePartGeometry::IsEmpty(const Geometry &geom) {
    D_ASSERT(GeometryTypes::IsSinglePart(geom.GetType()));
    return geom.data_count == 0;
}

//------------------------------------------------------------------------------
// MultiPartGeometry
//------------------------------------------------------------------------------
struct MultiPartGeometry {

    // static void Resize(Geometry &geom, ArenaAllocator &alloc, uint32_t new_count);

    static uint32_t PartCount(const Geometry &geom);
    static Geometry& Part(Geometry &geom, uint32_t index);
    static const Geometry& Part(const Geometry &geom, uint32_t index);
    static PartView Parts(Geometry &geom);
    static ConstPartView Parts(const Geometry &geom);

    static bool IsEmpty(const Geometry &geom) {
        D_ASSERT(GeometryTypes::IsMultiPart(geom.GetType()));
        for(uint32_t i = 0; i < geom.data_count; i++) {
            if(!Geometry::IsEmpty(Part(geom, i))) {
                return false;
            }
        }
        return true;
    }
};

inline uint32_t MultiPartGeometry::PartCount(const Geometry &geom) {
    D_ASSERT(GeometryTypes::IsMultiPart(geom.GetType()));
    return geom.data_count;
}

inline Geometry& MultiPartGeometry::Part(Geometry &geom, uint32_t index) {
    D_ASSERT(GeometryTypes::IsMultiPart(geom.GetType()));
    D_ASSERT(index < geom.data_count);
    return *reinterpret_cast<Geometry*>(geom.GetData() + index * sizeof(Geometry));
}

inline const Geometry& MultiPartGeometry::Part(const Geometry &geom, uint32_t index) {
    D_ASSERT(GeometryTypes::IsMultiPart(geom.GetType()));
    D_ASSERT(index < geom.data_count);
    return *reinterpret_cast<const Geometry*>(geom.GetData() + index * sizeof(Geometry));
}

inline PartView MultiPartGeometry::Parts(Geometry &geom) {
    D_ASSERT(GeometryTypes::IsMultiPart(geom.GetType()));
    auto ptr = reinterpret_cast<Geometry*>(geom.GetData());
    return { ptr, ptr + geom.data_count };
}

inline ConstPartView MultiPartGeometry::Parts(const Geometry &geom) {
    D_ASSERT(GeometryTypes::IsMultiPart(geom.GetType()));
    auto ptr = reinterpret_cast<const Geometry*>(geom.GetData());
    return { ptr, ptr + geom.data_count };
}

//------------------------------------------------------------------------------
// CollectionGeometry
//------------------------------------------------------------------------------
struct CollectionGeometry : public MultiPartGeometry {
protected:
    static Geometry Create(ArenaAllocator &alloc, GeometryType type, vector<Geometry> &items, bool has_z, bool has_m) {
        D_ASSERT(GeometryTypes::IsCollection(type));
        auto collection = Geometry::Create(alloc, type, items.size(), has_z, has_m);
        for(uint32_t i = 0; i < items.size(); i++) {
            CollectionGeometry::Part(collection, i) = std::move(items[i]);
        }
        return collection;
    }
};

//------------------------------------------------------------------------------
// Point
//------------------------------------------------------------------------------
struct Point : public SinglePartGeometry {
    static Geometry Create(ArenaAllocator &alloc, uint32_t count, bool has_z, bool has_m);
    static Geometry CreateEmpty(bool has_z, bool has_m);

    template<class V>
    static Geometry CreateFromVertex(ArenaAllocator &alloc, const V &vertex);

    static Geometry CreateFromCopy(ArenaAllocator &alloc, const_data_ptr_t data, uint32_t count, bool has_z, bool has_m) {
        auto point = Point::Create(alloc, 1, has_z, has_m);
        SinglePartGeometry::CopyData(point, alloc, data, count, has_z, has_m);
        return point;
    }

    // Methods
    template<class V = VertexXY>
    static V GetVertex(const Geometry &geom);

    template<class V = VertexXY>
    static void SetVertex(Geometry &geom, const V &vertex);

    // Constants
    static const constexpr GeometryType TYPE = GeometryType::POINT;
    static const constexpr int SURFACE_DIMENSION = 0;
};

inline Geometry Point::Create(ArenaAllocator &alloc, uint32_t count, bool has_z, bool has_m) {
    return Geometry::Create(alloc, TYPE, count, has_z, has_m);
}

inline Geometry Point::CreateEmpty(bool has_z, bool has_m) {
    return Geometry::CreateEmpty(TYPE, has_z, has_m);
}

template<class V>
inline Geometry Point::CreateFromVertex(ArenaAllocator &alloc, const V &vertex) {
    auto point = Create(alloc, 1, V::HAS_Z, V::HAS_M);
    Point::SetVertex(point, vertex);
    return point;
}

template<class V>
inline V Point::GetVertex(const Geometry &geom) {
    D_ASSERT(geom.GetType() == TYPE);
    D_ASSERT(geom.Count() == 1);
    D_ASSERT(geom.GetProperties().HasZ() == V::HAS_Z);
    D_ASSERT(geom.GetProperties().HasM() == V::HAS_M);
    return SinglePartGeometry::GetVertex<V>(geom, 0);
}

template<class V>
void Point::SetVertex(Geometry &geom, const V &vertex) {
    D_ASSERT(geom.GetType() == TYPE);
    D_ASSERT(geom.Count() == 1);
    D_ASSERT(geom.GetProperties().HasZ() == V::HAS_Z);
    D_ASSERT(geom.GetProperties().HasM() == V::HAS_M);
    SinglePartGeometry::SetVertex(geom, 0, vertex);
}

//------------------------------------------------------------------------------
// LineString
//------------------------------------------------------------------------------
struct LineString : public SinglePartGeometry {
    static Geometry Create(ArenaAllocator &alloc, uint32_t count, bool has_z, bool has_m);
    static Geometry CreateEmpty(bool has_z, bool has_m);

    static Geometry CreateFromCopy(ArenaAllocator &alloc, const_data_ptr_t data, uint32_t count, bool has_z, bool has_m) {
        auto line = LineString::Create(alloc, 1, has_z, has_m);
        SinglePartGeometry::CopyData(line, alloc, data, count, has_z, has_m);
        return line;
    }

    // TODO: Wrap
    // Create a new LineString referencing a slice of the this linestring
    static Geometry GetSliceAsReference(const Geometry &geom, uint32_t start, uint32_t count) {
        auto line = LineString::CreateEmpty(geom.GetProperties().HasZ(), geom.GetProperties().HasM());
        SinglePartGeometry::ReferenceData(line, geom, start, count);
        return line;
    }

    // TODO: Wrap
    // Create a new LineString referencing a single point in the this linestring
    static Geometry GetPointAsReference(const Geometry &geom, uint32_t index) {
        auto count = index >= geom.Count() ? 0 : 1;
        auto point = Point::CreateEmpty(geom.GetProperties().HasZ(), geom.GetProperties().HasM());
        SinglePartGeometry::ReferenceData(point, geom, index, count);
        return point;
    }

    // Constants
    static const constexpr GeometryType TYPE = GeometryType::LINESTRING;
    static const constexpr int SURFACE_DIMENSION = 1;
};

inline Geometry LineString::Create(ArenaAllocator &alloc, uint32_t count, bool has_z, bool has_m) {
    return Geometry::Create(alloc, TYPE, count, has_z, has_m);
}

inline Geometry LineString::CreateEmpty(bool has_z, bool has_m) {
    return Geometry::CreateEmpty(TYPE, has_z, has_m);
}

//------------------------------------------------------------------------------
// Polygon
//------------------------------------------------------------------------------
struct Polygon : public MultiPartGeometry {
    // Constructors
    static Geometry Create(ArenaAllocator &alloc, uint32_t count, bool has_z, bool has_m);
    static Geometry CreateEmpty(bool has_z, bool has_m);
    static Geometry CreateFromBox(ArenaAllocator &alloc, double minx, double miny, double maxx, double maxy);

    // Methods

    // Constants
    static const constexpr GeometryType TYPE = GeometryType::POLYGON;
    static const constexpr int SURFACE_DIMENSION = 2;
};

inline Geometry Polygon::Create(ArenaAllocator &alloc, uint32_t count, bool has_z, bool has_m) {
    auto geom = Geometry::Create(alloc, TYPE, count, has_z, has_m);
    for(uint32_t i = 0; i < count; i++) {
        // Placement new
        new (&Polygon::Part(geom, i)) Geometry(GeometryType::LINESTRING, has_z, has_m);
    }
    return geom;
}

inline Geometry Polygon::CreateEmpty(bool has_z, bool has_m) {
    return Geometry::CreateEmpty(TYPE, has_z, has_m);
}

inline Geometry Polygon::CreateFromBox(ArenaAllocator &alloc, double minx, double miny, double maxx, double maxy) {
    auto polygon = Polygon::Create(alloc, 1, false, false);
    auto &ring = Polygon::Part(polygon, 0);
    LineString::Resize(ring, alloc, 5);
    LineString::SetVertex(ring, 0, { minx, miny });
    LineString::SetVertex(ring, 1, { miny, maxy });
    LineString::SetVertex(ring, 2, { maxx, maxy });
    LineString::SetVertex(ring, 3, { maxx, miny });
    LineString::SetVertex(ring, 4, { minx, miny });
    return polygon;
}

//------------------------------------------------------------------------------
// MultiPoint
//------------------------------------------------------------------------------
struct MultiPoint : public CollectionGeometry {
    static Geometry Create(ArenaAllocator &alloc, uint32_t count, bool has_z, bool has_m);
    static Geometry CreateEmpty(bool has_z, bool has_m);
    static Geometry Create(ArenaAllocator &alloc, vector<Geometry> &items, bool has_z, bool has_m);

    // Constants
    static const constexpr GeometryType TYPE = GeometryType::MULTIPOINT;
    static const constexpr int SURFACE_DIMENSION = 0;
};

inline Geometry MultiPoint::Create(ArenaAllocator &alloc, uint32_t count, bool has_z, bool has_m) {
    auto geom = Geometry::Create(alloc, TYPE, count, has_z, has_m);
    for(uint32_t i = 0; i < count; i++) {
        // Placement new
        new (&MultiPoint::Part(geom, i)) Geometry(GeometryType::POINT, has_z, has_m);
    }
    return geom;
}

inline Geometry MultiPoint::CreateEmpty(bool has_z, bool has_m) {
    return Geometry::CreateEmpty(TYPE, has_z, has_m);
}

inline Geometry MultiPoint::Create(ArenaAllocator &alloc, vector<Geometry> &items, bool has_z, bool has_m) {
    return CollectionGeometry::Create(alloc, TYPE, items, has_z, has_m);
}

//------------------------------------------------------------------------------
// MultiLineString
//------------------------------------------------------------------------------
struct MultiLineString : public CollectionGeometry {
    static Geometry Create(ArenaAllocator &alloc, uint32_t count, bool has_z, bool has_m);
    static Geometry CreateEmpty(bool has_z, bool has_m);
    static Geometry Create(ArenaAllocator &alloc, vector<Geometry> &items, bool has_z, bool has_m);


    // Constants
    static const constexpr GeometryType TYPE = GeometryType::MULTILINESTRING;
    static const constexpr int SURFACE_DIMENSION = 1;
};

inline Geometry MultiLineString::Create(ArenaAllocator &alloc, uint32_t count, bool has_z, bool has_m) {
    auto geom = Geometry::Create(alloc, TYPE, count, has_z, has_m);
    for(uint32_t i = 0; i < count; i++) {
        // Placement new
        new (&MultiLineString::Part(geom, i)) Geometry(GeometryType::LINESTRING, has_z, has_m);
    }
    return geom;
}

inline Geometry MultiLineString::CreateEmpty(bool has_z, bool has_m) {
    return Geometry::CreateEmpty(TYPE, has_z, has_m);
}

inline Geometry MultiLineString::Create(ArenaAllocator &alloc, vector<Geometry> &items, bool has_z, bool has_m) {
    return CollectionGeometry::Create(alloc, TYPE, items, has_z, has_m);
}

//------------------------------------------------------------------------------
// MultiPolygon
//------------------------------------------------------------------------------
struct MultiPolygon : public CollectionGeometry {
    static Geometry Create(ArenaAllocator &alloc, uint32_t count, bool has_z, bool has_m);
    static Geometry CreateEmpty(bool has_z, bool has_m);
    static Geometry Create(ArenaAllocator &alloc, vector<Geometry> &items, bool has_z, bool has_m);

    // Constants
    static const constexpr GeometryType TYPE = GeometryType::MULTIPOLYGON;
    static const constexpr int SURFACE_DIMENSION = 2;
};

inline Geometry MultiPolygon::Create(ArenaAllocator &alloc, uint32_t count, bool has_z, bool has_m) {
    auto geom = Geometry::Create(alloc, TYPE, count, has_z, has_m);
    for(uint32_t i = 0; i < count; i++) {
        // Placement new
        new (&MultiPolygon::Part(geom, i)) Geometry(GeometryType::POLYGON, has_z, has_m);
    }
    return geom;
}

inline Geometry MultiPolygon::CreateEmpty(bool has_z, bool has_m) {
    return Geometry::CreateEmpty(TYPE, has_z, has_m);
}

inline Geometry MultiPolygon::Create(ArenaAllocator &alloc, vector<Geometry> &items, bool has_z, bool has_m) {
    return CollectionGeometry::Create(alloc, TYPE, items, has_z, has_m);
}

//------------------------------------------------------------------------------
// GeometryCollection
//------------------------------------------------------------------------------
struct GeometryCollection : public CollectionGeometry {
    static Geometry Create(ArenaAllocator &alloc, uint32_t count, bool has_z, bool has_m);
    static Geometry CreateEmpty(bool has_z, bool has_m);
    static Geometry Create(ArenaAllocator &alloc, vector<Geometry> &items, bool has_z, bool has_m);

    // Constants
    static const constexpr GeometryType TYPE = GeometryType::GEOMETRYCOLLECTION;
};

inline Geometry GeometryCollection::Create(ArenaAllocator &alloc, uint32_t count, bool has_z, bool has_m) {
    auto geom = Geometry::Create(alloc, TYPE, count, has_z, has_m);
    for(uint32_t i = 0; i < count; i++) {
        // Placement new
        new (&GeometryCollection::Part(geom, i)) Geometry(GeometryType::GEOMETRYCOLLECTION, has_z, has_m);
    }
    return geom;
}

inline Geometry GeometryCollection::CreateEmpty(bool has_z, bool has_m) {
    return Geometry::CreateEmpty(TYPE, has_z, has_m);
}

inline Geometry GeometryCollection::Create(ArenaAllocator &alloc, vector<Geometry> &items, bool has_z, bool has_m) {
    return CollectionGeometry::Create(alloc, TYPE, items, has_z, has_m);
}

//------------------------------------------------------------------------------
// Inlined Functions
//------------------------------------------------------------------------------
template<class FUNC>
inline void Geometry::ExtractPoints(const Geometry &geom, FUNC &&func) {
    struct op {
        static void Case(Geometry::Tags::Point, const Geometry &geom, FUNC &&func) {
            func(geom);
        }
        static void Case(Geometry::Tags::MultiPoint, const Geometry &geom, FUNC &&func) {
            auto parts = MultiPoint::Parts(geom);
            for(auto &part : parts) {
                func(part);
            }
        }
        static void Case(Geometry::Tags::GeometryCollection, const Geometry &geom, FUNC &&func) {
            auto parts = GeometryCollection::Parts(geom);
            for(auto &part : parts) {
                Visit<op>(part, std::forward<FUNC>(func));
            }
        }
        static void Case(Geometry::Tags::AnyGeometry, const Geometry&, FUNC &&) { }
    };
    Visit<op>(geom, std::forward<FUNC>(func));
}

template<class FUNC>
inline void Geometry::ExtractLines(const Geometry &geom, FUNC &&func) {
    struct op {
        static void Case(Geometry::Tags::LineString, const Geometry &geom, FUNC &&func) {
            func(geom);
        }
        static void Case(Geometry::Tags::MultiLineString, const Geometry &geom, FUNC &&func) {
            auto parts = MultiLineString::Parts(geom);
            for(auto &part : parts) {
                func(part);
            }
        }
        static void Case(Geometry::Tags::GeometryCollection, const Geometry &geom, FUNC &&func) {
            auto parts = GeometryCollection::Parts(geom);
            for(auto &part : parts) {
                Visit<op>(part, std::forward<FUNC>(func));
            }
        }
        static void Case(Geometry::Tags::AnyGeometry, const Geometry&, FUNC &&) { }
    };
    Visit<op>(geom, std::forward<FUNC>(func));
}

template<class FUNC>
inline void Geometry::ExtractPolygons(const Geometry &geom, FUNC &&func){
    struct op {
        static void Case(Geometry::Tags::Polygon, const Geometry &geom, FUNC &&func) {
            func(geom);
        }
        static void Case(Geometry::Tags::MultiPolygon, const Geometry &geom, FUNC &&func) {
            auto parts = MultiPolygon::Parts(geom);
            for(auto &part : parts) {
                func(part);
            }
        }
        static void Case(Geometry::Tags::GeometryCollection, const Geometry &geom, FUNC &&func) {
            auto parts = GeometryCollection::Parts(geom);
            for(auto &part : parts) {
                Visit<op>(part, std::forward<FUNC>(func));
            }
        }
        static void Case(Geometry::Tags::AnyGeometry, const Geometry&, FUNC &&) { }
    };
    Visit<op>(geom, std::forward<FUNC>(func));
}

//------------------------------------------------------------------------------
// Assertions
//------------------------------------------------------------------------------

static_assert(std::is_standard_layout<Geometry>::value, "Geometry must be standard layout");

} // namespace core

} // namespace spatial