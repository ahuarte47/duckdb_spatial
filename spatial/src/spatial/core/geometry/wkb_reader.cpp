#include "spatial/common.hpp"
#include "spatial/core/geometry/vertex_vector.hpp"
#include "spatial/core/geometry/geometry.hpp"
#include "spatial/core/geometry/wkb_reader.hpp"
#include "spatial/core/geometry/geometry_factory.hpp"

namespace spatial {

namespace core {

template <>
uint32_t WKBReader::ReadInt<WKBByteOrder::NDR>() {
	if(cursor + sizeof(uint32_t) > length) {
		throw SerializationException("WKBReader: ReadInt: not enough data");
	}
	// Read uint32_t in little endian
	auto result = Load<uint32_t>((const_data_ptr_t)data + cursor);
	cursor += sizeof(uint32_t);
	return result;
}

template <>
double WKBReader::ReadDouble<WKBByteOrder::NDR>() {
	if (cursor + sizeof(double) > length) {
		throw SerializationException("WKBReader: ReadDouble: not enough data");
	}
	// Read double in little endian
	auto result = Load<double>((const_data_ptr_t)data + cursor);
	cursor += sizeof(double);
	return result;
}

template <>
uint32_t WKBReader::ReadInt<WKBByteOrder::XDR>() {
	if(cursor + sizeof(uint32_t) > length) {
		throw SerializationException("WKBReader: ReadInt: not enough data");
	}
	// Read uint32_t in big endian
	uint32_t result = 0;
	result |= (uint32_t)data[cursor + 0] << 24 & 0xFF000000;
	result |= (uint32_t)data[cursor + 1] << 16 & 0x00FF0000;
	result |= (uint32_t)data[cursor + 2] << 8 & 0x0000FF00;
	result |= (uint32_t)data[cursor + 3] << 0 & 0x000000FF;
	cursor += sizeof(uint32_t);
	return result;
}

template <>
double WKBReader::ReadDouble<WKBByteOrder::XDR>() {
	if(cursor + sizeof(double) > length) {
		throw SerializationException("WKBReader: ReadDouble: not enough data");
	}
	// Read double in big endian
	uint64_t result = 0;
	result |= (uint64_t)data[cursor + 0] << 56 & 0xFF00000000000000;
	result |= (uint64_t)data[cursor + 1] << 48 & 0x00FF000000000000;
	result |= (uint64_t)data[cursor + 2] << 40 & 0x0000FF0000000000;
	result |= (uint64_t)data[cursor + 3] << 32 & 0x000000FF00000000;
	result |= (uint64_t)data[cursor + 4] << 24 & 0x00000000FF000000;
	result |= (uint64_t)data[cursor + 5] << 16 & 0x0000000000FF0000;
	result |= (uint64_t)data[cursor + 6] << 8 & 0x000000000000FF00;
	result |= (uint64_t)data[cursor + 7] << 0 & 0x00000000000000FF;
	cursor += sizeof(uint64_t);
	double d;
	memcpy(&d, &result, sizeof(double));
	return d;
}

Geometry WKBReader::ReadGeometry() {
	auto order = static_cast<WKBByteOrder>(data[cursor++]);
	if (order == WKBByteOrder::XDR) {
		return ReadGeometryImpl<WKBByteOrder::XDR>();
	} else {
		return ReadGeometryImpl<WKBByteOrder::NDR>();
	}
}

Point WKBReader::ReadPoint() {
	auto order = static_cast<WKBByteOrder>(data[cursor++]);
	if (order == WKBByteOrder::XDR) {
		auto type = (WKBGeometryType)ReadInt<WKBByteOrder::XDR>();
		D_ASSERT(type == WKBGeometryType::POINT);
		(void)type;
		return ReadPointImpl<WKBByteOrder::XDR>();
	} else {
		auto type = (WKBGeometryType)ReadInt<WKBByteOrder::NDR>();
		D_ASSERT(type == WKBGeometryType::POINT);
		(void)type;
		return ReadPointImpl<WKBByteOrder::NDR>();
	}
}

LineString WKBReader::ReadLineString() {
	auto order = static_cast<WKBByteOrder>(data[cursor++]);
	if (order == WKBByteOrder::XDR) {
		auto type = (WKBGeometryType)ReadInt<WKBByteOrder::XDR>();
		D_ASSERT(type == WKBGeometryType::LINESTRING);
		(void)type;
		return ReadLineStringImpl<WKBByteOrder::XDR>();
	} else {
		auto type = (WKBGeometryType)ReadInt<WKBByteOrder::NDR>();
		D_ASSERT(type == WKBGeometryType::LINESTRING);
		(void)type;
		return ReadLineStringImpl<WKBByteOrder::NDR>();
	}
}

Polygon WKBReader::ReadPolygon() {
	auto order = static_cast<WKBByteOrder>(data[cursor++]);
	if (order == WKBByteOrder::XDR) {
		auto type = (WKBGeometryType)ReadInt<WKBByteOrder::XDR>();
		D_ASSERT(type == WKBGeometryType::POLYGON);
		(void)type;
		return ReadPolygonImpl<WKBByteOrder::XDR>();
	} else {
		auto type = (WKBGeometryType)ReadInt<WKBByteOrder::NDR>();
		D_ASSERT(type == WKBGeometryType::POLYGON);
		(void)type;
		return ReadPolygonImpl<WKBByteOrder::NDR>();
	}
}

MultiPoint WKBReader::ReadMultiPoint() {
	auto order = static_cast<WKBByteOrder>(data[cursor++]);
	if (order == WKBByteOrder::XDR) {
		auto type = (WKBGeometryType)ReadInt<WKBByteOrder::XDR>();
		D_ASSERT(type == WKBGeometryType::MULTIPOINT);
		(void)type;
		return ReadMultiPointImpl<WKBByteOrder::XDR>();
	} else {
		auto type = (WKBGeometryType)ReadInt<WKBByteOrder::NDR>();
		D_ASSERT(type == WKBGeometryType::MULTIPOINT);
		(void)type;
		return ReadMultiPointImpl<WKBByteOrder::NDR>();
	}
}

MultiLineString WKBReader::ReadMultiLineString() {
	auto order = static_cast<WKBByteOrder>(data[cursor++]);
	if (order == WKBByteOrder::XDR) {
		auto type = (WKBGeometryType)ReadInt<WKBByteOrder::XDR>();
		D_ASSERT(type == WKBGeometryType::MULTILINESTRING);
		(void)type;
		return ReadMultiLineStringImpl<WKBByteOrder::XDR>();
	} else {
		auto type = (WKBGeometryType)ReadInt<WKBByteOrder::NDR>();
		D_ASSERT(type == WKBGeometryType::MULTILINESTRING);
		(void)type;
		return ReadMultiLineStringImpl<WKBByteOrder::NDR>();
	}
}

MultiPolygon WKBReader::ReadMultiPolygon() {
	auto order = static_cast<WKBByteOrder>(data[cursor++]);
	if (order == WKBByteOrder::XDR) {
		auto type = (WKBGeometryType)ReadInt<WKBByteOrder::XDR>();
		D_ASSERT(type == WKBGeometryType::MULTIPOLYGON);
		(void)type;
		return ReadMultiPolygonImpl<WKBByteOrder::XDR>();
	} else {
		auto type = (WKBGeometryType)ReadInt<WKBByteOrder::NDR>();
		D_ASSERT(type == WKBGeometryType::MULTIPOLYGON);
		(void)type;
		return ReadMultiPolygonImpl<WKBByteOrder::NDR>();
	}
}

GeometryCollection WKBReader::ReadGeometryCollection() {
	auto order = static_cast<WKBByteOrder>(data[cursor++]);
	if (order == WKBByteOrder::XDR) {
		auto type = (WKBGeometryType)ReadInt<WKBByteOrder::XDR>();
		D_ASSERT(type == WKBGeometryType::GEOMETRYCOLLECTION);
		(void)type;
		return ReadGeometryCollectionImpl<WKBByteOrder::XDR>();
	} else {
		auto type = (WKBGeometryType)ReadInt<WKBByteOrder::NDR>();
		D_ASSERT(type == WKBGeometryType::GEOMETRYCOLLECTION);
		(void)type;
		return ReadGeometryCollectionImpl<WKBByteOrder::NDR>();
	}
}

template <WKBByteOrder ORDER>
Geometry WKBReader::ReadGeometryImpl() {
	auto type = (WKBGeometryType)ReadInt<ORDER>();
	switch (type) {
	case WKBGeometryType::POINT:
		return Geometry(ReadPointImpl<ORDER>());
	case WKBGeometryType::LINESTRING:
		return Geometry(ReadLineStringImpl<ORDER>());
	case WKBGeometryType::POLYGON:
		return Geometry(ReadPolygonImpl<ORDER>());
	case WKBGeometryType::MULTIPOINT:
		return Geometry(ReadMultiPointImpl<ORDER>());
	case WKBGeometryType::MULTILINESTRING:
		return Geometry(ReadMultiLineStringImpl<ORDER>());
	case WKBGeometryType::MULTIPOLYGON:
		return Geometry(ReadMultiPolygonImpl<ORDER>());
	case WKBGeometryType::GEOMETRYCOLLECTION:
		return Geometry(ReadGeometryCollectionImpl<ORDER>());
	default:
		throw NotImplementedException("Geometry type not supported");
	}
}

template <WKBByteOrder ORDER>
Point WKBReader::ReadPointImpl() {
	auto x = ReadDouble<ORDER>();
	auto y = ReadDouble<ORDER>();
	if (std::isnan(x) && std::isnan(y)) {
		auto point_data = factory.AllocateVertexVector(0);
		return Point(point_data);
	}
	auto point_data = factory.AllocateVertexVector(1);
	point_data.Add(Vertex(x, y));
	return Point(point_data);
}

template <WKBByteOrder ORDER>
LineString WKBReader::ReadLineStringImpl() {
	auto num_points = ReadInt<ORDER>();
	auto line_data = factory.AllocateVertexVector(num_points);
	for (uint32_t i = 0; i < num_points; i++) {
		auto x = ReadDouble<ORDER>();
		auto y = ReadDouble<ORDER>();
		line_data.Add(Vertex(x, y));
	}
	return LineString(line_data);
}

template <WKBByteOrder ORDER>
Polygon WKBReader::ReadPolygonImpl() {
	auto num_rings = ReadInt<ORDER>();
	auto rings = reinterpret_cast<VertexVector *>(factory.allocator.Allocate(sizeof(VertexVector) * num_rings));

	for (uint32_t i = 0; i < num_rings; i++) {
		auto num_points = ReadInt<ORDER>();
		rings[i] = factory.AllocateVertexVector(num_points);
		auto &ring = rings[i];

		for (uint32_t j = 0; j < num_points; j++) {
			auto x = ReadDouble<ORDER>();
			auto y = ReadDouble<ORDER>();
			ring.Add(Vertex(x, y));
		}
	}
	return Polygon(rings, num_rings);
}

template <WKBByteOrder ORDER>
MultiPoint WKBReader::ReadMultiPointImpl() {
	auto num_points = ReadInt<ORDER>();
	auto points = reinterpret_cast<Point *>(factory.allocator.Allocate(sizeof(Point) * num_points));
	for (uint32_t i = 0; i < num_points; i++) {
		points[i] = ReadPoint();
	}
	return MultiPoint(points, num_points);
}

template <WKBByteOrder ORDER>
MultiLineString WKBReader::ReadMultiLineStringImpl() {
	auto num_lines = ReadInt<ORDER>();
	auto lines = reinterpret_cast<LineString *>(factory.allocator.Allocate(sizeof(LineString) * num_lines));
	for (uint32_t i = 0; i < num_lines; i++) {
		lines[i] = ReadLineString();
	}
	return MultiLineString(lines, num_lines);
}

template <WKBByteOrder ORDER>
MultiPolygon WKBReader::ReadMultiPolygonImpl() {
	auto num_polygons = ReadInt<ORDER>();
	auto polygons = reinterpret_cast<Polygon *>(factory.allocator.Allocate(sizeof(Polygon) * num_polygons));
	for (uint32_t i = 0; i < num_polygons; i++) {
		polygons[i] = ReadPolygon();
	}
	return MultiPolygon(polygons, num_polygons);
}

template <WKBByteOrder ORDER>
GeometryCollection WKBReader::ReadGeometryCollectionImpl() {
	auto num_geometries = ReadInt<ORDER>();
	auto geometries = reinterpret_cast<Geometry *>(factory.allocator.Allocate(sizeof(Geometry) * num_geometries));
	for (uint32_t i = 0; i < num_geometries; i++) {
		geometries[i] = ReadGeometry();
	}
	return GeometryCollection(geometries, num_geometries);
}

} // namespace core

} // namespace spatial
