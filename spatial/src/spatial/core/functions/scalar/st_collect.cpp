#include "spatial/common.hpp"
#include "spatial/core/types.hpp"
#include "spatial/core/functions/scalar.hpp"
#include "spatial/core/functions/common.hpp"
#include "spatial/core/geometry/geometry.hpp"

#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"
#include "duckdb/common/vector_operations/unary_executor.hpp"
#include "duckdb/common/vector_operations/binary_executor.hpp"

namespace spatial {

namespace core {

static void CollectFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &lstate = GeometryFunctionLocalState::ResetAndGet(state);
	auto &arena = lstate.factory.allocator;
	auto count = args.size();
	auto &child_vec = ListVector::GetEntry(args.data[0]);
	UnifiedVectorFormat format;
	child_vec.ToUnifiedFormat(count, format);

	UnaryExecutor::Execute<list_entry_t, geometry_t>(args.data[0], result, count, [&](list_entry_t &geometry_list) {
		auto offset = geometry_list.offset;
		auto length = geometry_list.length;

		// First figure out if we have Z or M
		bool has_z = false;
		bool has_m = false;
		for (idx_t i = offset; i < offset + length; i++) {
			auto mapped_idx = format.sel->get_index(i);
			if (format.validity.RowIsValid(mapped_idx)) {
				auto geometry_blob = ((geometry_t *)format.data)[mapped_idx];
				auto props = geometry_blob.GetProperties();
				has_z = has_z || props.HasZ();
				has_m = has_m || props.HasM();
			}
		}

		// TODO: Peek the types first
		vector<Geometry> geometries;
		for (idx_t i = offset; i < offset + length; i++) {
			auto mapped_idx = format.sel->get_index(i);
			if (format.validity.RowIsValid(mapped_idx)) {
				auto geometry_blob = ((geometry_t *)format.data)[mapped_idx];
				auto geometry = lstate.factory.Deserialize(geometry_blob);
				// Dont add empty geometries
				if (!geometry.IsEmpty()) {
					geometries.push_back(geometry);
				}
			}
		}

		if (geometries.empty()) {
			GeometryCollection empty(has_z, has_m);
			return lstate.factory.Serialize(result, empty, has_z, has_m);
		}

		bool all_points = true;
		bool all_lines = true;
		bool all_polygons = true;

		for (auto &geometry : geometries) {
			if (geometry.Type() != GeometryType::POINT) {
				all_points = false;
			}
			if (geometry.Type() != GeometryType::LINESTRING) {
				all_lines = false;
			}
			if (geometry.Type() != GeometryType::POLYGON) {
				all_polygons = false;
			}
		}

		// TODO: Dont upcast the children, just append them.

		if (all_points) {
			MultiPoint collection(arena, geometries.size(), has_z, has_m);
			for (idx_t i = 0; i < geometries.size(); i++) {
				collection[i] = geometries[i].SetVertexType(arena, has_z, has_m).As<Point>();
			}
			return lstate.factory.Serialize(result, collection, has_z, has_m);
		} else if (all_lines) {
			MultiLineString collection(arena, geometries.size(), has_z, has_m);
			for (idx_t i = 0; i < geometries.size(); i++) {
				collection[i] = geometries[i].SetVertexType(arena, has_z, has_m).As<LineString>();
			}
			return lstate.factory.Serialize(result, collection, has_z, has_m);
		} else if (all_polygons) {
			MultiPolygon collection(arena, geometries.size(), has_z, has_m);
			for (idx_t i = 0; i < geometries.size(); i++) {
				collection[i] = geometries[i].SetVertexType(arena, has_z, has_m).As<Polygon>();
			}
			return lstate.factory.Serialize(result, collection, has_z, has_m);
		} else {
			GeometryCollection collection(arena, geometries.size(), has_z, has_m);
			for (idx_t i = 0; i < geometries.size(); i++) {
				collection[i] = geometries[i].SetVertexType(arena, has_z, has_m);
			}
			return lstate.factory.Serialize(result, collection, has_z, has_m);
		}
	});
}

//------------------------------------------------------------------------------
// Documentation
//------------------------------------------------------------------------------

static constexpr const char *DOC_DESCRIPTION = R"(
Collects geometries into a collection geometry

Collects a list of geometries into a collection geometry.
- If all geometries are `POINT`'s, a `MULTIPOINT` is returned.
- If all geometries are `LINESTRING`'s, a `MULTILINESTRING` is returned.
- If all geometries are `POLYGON`'s, a `MULTIPOLYGON` is returned.
- Otherwise if the input collection contains a mix of geometry types, a `GEOMETRYCOLLECTION` is returned.

Empty and `NULL` geometries are ignored. If all geometries are empty or `NULL`, a `GEOMETRYCOLLECTION EMPTY` is returned.
)";

static constexpr const char *DOC_EXAMPLE = R"(
-- With all POINT's, a MULTIPOINT is returned
SELECT ST_Collect([ST_Point(1, 2), ST_Point(3, 4)]);
----
MULTIPOINT (1 2, 3 4)

-- With mixed geometry types, a GEOMETRYCOLLECTION is returned
SELECT ST_Collect([ST_Point(1, 2), ST_GeomFromText('LINESTRING(3 4, 5 6)')]);
----
GEOMETRYCOLLECTION (POINT (1 2), LINESTRING (3 4, 5 6))

-- Note that the empty geometry is ignored, so the result is a MULTIPOINT
SELECT ST_Collect([ST_Point(1, 2), NULL, ST_GeomFromText('GEOMETRYCOLLECTION EMPTY')]);
----
MULTIPOINT (1 2)

-- If all geometries are empty or NULL, a GEOMETRYCOLLECTION EMPTY is returned
SELECT ST_Collect([NULL, ST_GeomFromText('GEOMETRYCOLLECTION EMPTY')]);
----
GEOMETRYCOLLECTION EMPTY

-- Tip: You can use the `ST_Collect` function together with the `list()` aggregate function to collect multiple rows of geometries into a single geometry collection:

CREATE TABLE points (geom GEOMETRY);

INSERT INTO points VALUES (ST_Point(1, 2)), (ST_Point(3, 4));

SELECT ST_Collect(list(geom)) FROM points;
----
MULTIPOINT (1 2, 3 4)
)";

static constexpr DocTag DOC_TAGS[] = {{"ext", "spatial"}, {"category", "construction"}};
//------------------------------------------------------------------------------
// Register functions
//------------------------------------------------------------------------------

void CoreScalarFunctions::RegisterStCollect(DatabaseInstance &db) {
	ScalarFunctionSet set("ST_Collect");

	set.AddFunction(ScalarFunction({LogicalType::LIST(GeoTypes::GEOMETRY())}, GeoTypes::GEOMETRY(), CollectFunction,
	                               nullptr, nullptr, nullptr, GeometryFunctionLocalState::Init));

	ExtensionUtil::RegisterFunction(db, set);
	DocUtil::AddDocumentation(db, "ST_Collect", DOC_DESCRIPTION, DOC_EXAMPLE, DOC_TAGS);
}

} // namespace core

} // namespace spatial
