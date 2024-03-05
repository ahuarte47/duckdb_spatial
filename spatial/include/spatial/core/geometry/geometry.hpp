#pragma once

#include "spatial/common.hpp"
#include "spatial/core/geometry/geometry_properties.hpp"
#include "spatial/core/geometry/vertex_vector.hpp"
#include "spatial/core/geometry/cursor.hpp"
#include "spatial/core/geometry/geometry_type.hpp"

namespace spatial {

namespace core {

// A serialized geometry
class geometry_t {
private:
	string_t data;

public:
	geometry_t() = default;
	// NOLINTNEXTLINE
	explicit geometry_t(string_t data) : data(data) {
	}

	// NOLINTNEXTLINE
	operator string_t() const {
		return data;
	}

	GeometryType GetType() const {
		return Load<GeometryType>(const_data_ptr_cast(data.GetPrefix()));
	}
	GeometryProperties GetProperties() const {
		return Load<GeometryProperties>(const_data_ptr_cast(data.GetPrefix() + 1));
	}
	uint16_t GetHash() const {
		return Load<uint16_t>(const_data_ptr_cast(data.GetPrefix() + 2));
	}
};

static_assert(sizeof(geometry_t) == sizeof(string_t), "geometry_t should be the same size as string_t");

//------------------------------------------------------------------------------
// Geometry Objects
//------------------------------------------------------------------------------

template <class T>
class IteratorPair {
	T *begin_ptr;
	T *end_ptr;

public:
	IteratorPair(T *begin_ptr, T *end_ptr) : begin_ptr(begin_ptr), end_ptr(end_ptr) {
	}

	T *begin() {
		return begin_ptr;
	}

	T *end() {
		return end_ptr;
	}
};

template <class T>
class ConstIteratorPair {
	const T *begin_ptr;
	const T *end_ptr;

public:
	ConstIteratorPair(const T *begin_ptr, const T *end_ptr) : begin_ptr(begin_ptr), end_ptr(end_ptr) {
	}

	const T *begin() {
		return begin_ptr;
	}

	const T *end() {
		return end_ptr;
	}
};

struct Utils {
	static string format_coord(double d);
	static string format_coord(double x, double y);
	static string format_coord(double x, double y, double z);
	static string format_coord(double x, double y, double z, double m);

	static inline float DoubleToFloatDown(double d) {
		if (d > static_cast<double>(std::numeric_limits<float>::max())) {
			return std::numeric_limits<float>::max();
		}
		if (d <= static_cast<double>(std::numeric_limits<float>::lowest())) {
			return std::numeric_limits<float>::lowest();
		}

		auto f = static_cast<float>(d);
		if (static_cast<double>(f) <= d) {
			return f;
		}
		return std::nextafter(f, std::numeric_limits<float>::lowest());
	}

	static inline float DoubleToFloatUp(double d) {
		if (d >= static_cast<double>(std::numeric_limits<float>::max())) {
			return std::numeric_limits<float>::max();
		}
		if (d < static_cast<double>(std::numeric_limits<float>::lowest())) {
			return std::numeric_limits<float>::lowest();
		}

		auto f = static_cast<float>(d);
		if (static_cast<double>(f) >= d) {
			return f;
		}
		return std::nextafter(f, std::numeric_limits<float>::max());
	}
};

struct BoundingBox {
	double minx = std::numeric_limits<double>::max();
	double miny = std::numeric_limits<double>::max();
	double maxx = std::numeric_limits<double>::lowest();
	double maxy = std::numeric_limits<double>::lowest();

	bool Intersects(const BoundingBox &other) const {
		return !(minx > other.maxx || maxx < other.minx || miny > other.maxy || maxy < other.miny);
	}
};

struct Geometry;
struct GeometryFactory;

class Point {
	friend GeometryFactory;
	VertexVector vertices;

public:
	explicit Point(VertexVector vertices) : vertices(vertices) {
	}
	string ToString() const;
	bool IsEmpty() const;
	Vertex GetVertex() const;

	const VertexVector &Vertices() const {
		return vertices;
	}
	VertexVector &Vertices() {
		return vertices;
	}

	operator Geometry() const;
};

class LineString {
	friend GeometryFactory;
	VertexVector vertices;

public:
	explicit LineString(VertexVector vertices) : vertices(vertices) {
	}
	VertexVector &Vertices() {
		return vertices;
	}
	const VertexVector &Vertices() const {
		return vertices;
	}
	// Common Methods
	string ToString() const;
	// Geometry Methods
	bool IsEmpty() const;
	double Length() const;
	Geometry Centroid() const;
	uint32_t Count() const;

	operator Geometry() const;
};

class Polygon {
	friend GeometryFactory;
	VertexVector *rings;
	uint32_t num_rings;

public:
	explicit Polygon(VertexVector *rings, uint32_t num_rings) : rings(rings), num_rings(num_rings) {
	}

	VertexVector &Ring(uint32_t index) {
		D_ASSERT(index < num_rings);
		return rings[index];
	}
	const VertexVector &Ring(uint32_t index) const {
		D_ASSERT(index < num_rings);
		return rings[index];
	}
	const VertexVector &Shell() const {
		return rings[0];
	}
	VertexVector &Shell() {
		return rings[0];
	}

	IteratorPair<VertexVector> Rings() {
		return IteratorPair<VertexVector>(rings, rings + num_rings);
	}

	ConstIteratorPair<VertexVector> Rings() const {
		return ConstIteratorPair<VertexVector>(rings, rings + num_rings);
	}

	// Common Methods
	string ToString() const;
	// Geometry Methods
	double Area() const;
	bool IsEmpty() const;
	double Perimiter() const;
	Geometry Centroid() const;
	uint32_t Count() const;

	operator Geometry() const;
};

class MultiPoint {
	friend GeometryFactory;
	Point *points;
	uint32_t num_points;

public:
	explicit MultiPoint(Point *points, uint32_t num_points) : points(points), num_points(num_points) {
	}
	string ToString() const;

	// Collection Methods
	uint32_t Count() const;
	bool IsEmpty() const;
	Point &operator[](uint32_t index);
	const Point &operator[](uint32_t index) const;

	// Iterator Methods
	const Point *begin() const;
	const Point *end() const;
	Point *begin();
	Point *end();

	operator Geometry() const;
};

class MultiLineString {
	friend GeometryFactory;
	LineString *lines;
	uint32_t count;

public:
	explicit MultiLineString(LineString *lines, uint32_t count) : lines(lines), count(count) {
	}
	string ToString() const;

	// Geometry Methods
	double Length() const;

	// Collection Methods
	uint32_t Count() const;
	bool IsEmpty() const;
	LineString &operator[](uint32_t index);
	const LineString &operator[](uint32_t index) const;

	// Iterator Methods
	const LineString *begin() const;
	const LineString *end() const;
	LineString *begin();
	LineString *end();

	operator Geometry() const;
};

class MultiPolygon {
	friend GeometryFactory;
	Polygon *polygons;
	uint32_t count;

public:
	explicit MultiPolygon(Polygon *polygons, uint32_t count) : polygons(polygons), count(count) {
	}
	double Area() const;
	string ToString() const;

	// Collection Methods
	uint32_t Count() const;
	bool IsEmpty() const;
	Polygon &operator[](uint32_t index);
	const Polygon &operator[](uint32_t index) const;

	// Iterator Methods
	const Polygon *begin() const;
	const Polygon *end() const;
	Polygon *begin();
	Polygon *end();

	operator Geometry() const;
};

class GeometryCollection {
	friend GeometryFactory;
	Geometry *geometries;
	uint32_t count;

public:
	explicit GeometryCollection(Geometry *geometries, uint32_t count) : geometries(geometries), count(count) {
	}
	string ToString() const;

	// Collection Methods
	uint32_t Count() const;
	bool IsEmpty() const;
	Geometry &operator[](uint32_t index);
	const Geometry &operator[](uint32_t index) const;

	// Iterator Methods
	const Geometry *begin() const;
	const Geometry *end() const;
	Geometry *begin();
	Geometry *end();

	template <class AGG, class RESULT_TYPE>
	RESULT_TYPE Aggregate(AGG agg, RESULT_TYPE zero) const;

	operator Geometry() const;
};

struct Geometry {
	friend GeometryFactory;

private:
	GeometryType type;
	GeometryProperties properties;
	union {
		Point point;
		LineString linestring;
		Polygon polygon;
		MultiPoint multipoint;
		MultiLineString multilinestring;
		MultiPolygon multipolygon;
		GeometryCollection geometrycollection;
	};

public:
	explicit Geometry(Point point) : type(GeometryType::POINT), point(point) {
	}
	explicit Geometry(LineString linestring) : type(GeometryType::LINESTRING), linestring(linestring) {
	}
	explicit Geometry(Polygon polygon) : type(GeometryType::POLYGON), polygon(polygon) {
	}
	explicit Geometry(MultiPoint multipoint) : type(GeometryType::MULTIPOINT), multipoint(multipoint) {
	}
	explicit Geometry(MultiLineString multilinestring)
	    : type(GeometryType::MULTILINESTRING), multilinestring(multilinestring) {
	}
	explicit Geometry(MultiPolygon multipolygon) : type(GeometryType::MULTIPOLYGON), multipolygon(multipolygon) {
	}
	explicit Geometry(GeometryCollection geometrycollection)
	    : type(GeometryType::GEOMETRYCOLLECTION), geometrycollection(geometrycollection) {
	}

	// Accessor methods
	inline GeometryType Type() const {
		return type;
	}

	inline GeometryProperties &Properties() {
		return properties;
	}

	inline const GeometryProperties &Properties() const {
		return properties;
	}

	inline Point &GetPoint() {
		D_ASSERT(type == GeometryType::POINT);
		return point;
	}

	inline Point const &GetPoint() const {
		D_ASSERT(type == GeometryType::POINT);
		return point;
	}

	inline LineString &GetLineString() {
		D_ASSERT(type == GeometryType::LINESTRING);
		return linestring;
	}

	inline LineString const &GetLineString() const {
		D_ASSERT(type == GeometryType::LINESTRING);
		return linestring;
	}

	inline Polygon &GetPolygon() {
		D_ASSERT(type == GeometryType::POLYGON);
		return polygon;
	}

	inline Polygon const &GetPolygon() const {
		D_ASSERT(type == GeometryType::POLYGON);
		return polygon;
	}

	inline MultiPoint &GetMultiPoint() {
		D_ASSERT(type == GeometryType::MULTIPOINT);
		return multipoint;
	}

	inline MultiPoint const &GetMultiPoint() const {
		D_ASSERT(type == GeometryType::MULTIPOINT);
		return multipoint;
	}

	inline MultiLineString &GetMultiLineString() {
		D_ASSERT(type == GeometryType::MULTILINESTRING);
		return multilinestring;
	}

	inline MultiLineString const &GetMultiLineString() const {
		D_ASSERT(type == GeometryType::MULTILINESTRING);
		return multilinestring;
	}

	inline MultiPolygon &GetMultiPolygon() {
		D_ASSERT(type == GeometryType::MULTIPOLYGON);
		return multipolygon;
	}

	inline MultiPolygon const &GetMultiPolygon() const {
		D_ASSERT(type == GeometryType::MULTIPOLYGON);
		return multipolygon;
	}

	inline GeometryCollection &GetGeometryCollection() {
		D_ASSERT(type == GeometryType::GEOMETRYCOLLECTION);
		return geometrycollection;
	}

	inline GeometryCollection const &GetGeometryCollection() const {
		D_ASSERT(type == GeometryType::GEOMETRYCOLLECTION);
		return geometrycollection;
	}

	string ToString() const;
	int32_t Dimension() const;
	bool IsEmpty() const;
	bool IsCollection() const;
};

template <class AGG, class RESULT_TYPE>
RESULT_TYPE GeometryCollection::Aggregate(AGG agg, RESULT_TYPE zero) const {
	RESULT_TYPE result = zero;
	for (idx_t i = 0; i < count; i++) {
		auto &geometry = geometries[i];
		if (geometry.Type() == GeometryType::GEOMETRYCOLLECTION) {
			result = geometry.GetGeometryCollection().Aggregate(agg, result);
		} else {
			result = agg(geometry, result);
		}
	}
	return result;
}

} // namespace core

} // namespace spatial