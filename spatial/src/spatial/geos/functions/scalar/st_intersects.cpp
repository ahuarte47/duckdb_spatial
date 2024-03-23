#include "spatial/common.hpp"
#include "spatial/core/types.hpp"
#include "spatial/geos/functions/scalar.hpp"
#include "spatial/geos/functions/common.hpp"
#include "spatial/geos/geos_wrappers.hpp"
#include "spatial/geos/geos_executor.hpp"

#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"
#include "duckdb/common/vector_operations/unary_executor.hpp"
#include "duckdb/common/vector_operations/binary_executor.hpp"

namespace spatial {

namespace geos {

using namespace spatial::core;

static void IntersectsFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &lstate = GEOSFunctionLocalState::ResetAndGet(state);
	auto &left = args.data[0];
	auto &right = args.data[1];
	auto count = args.size();
	GEOSExecutor::ExecuteSymmetricPreparedBinary(lstate, left, right, count, result, GEOSIntersects_r,
	                                             GEOSPreparedIntersects_r);
}

//------------------------------------------------------------------------------
// Documentation
//------------------------------------------------------------------------------
static constexpr const char* DOC_DESCRIPTION = R"(
    Returns true if two geometries intersects
)";

static constexpr const char* DOC_EXAMPLE = R"(

)";


static constexpr DocTag DOC_TAGS[] = {{"ext", "spatial"}, {"category", "property"}};
//------------------------------------------------------------------------------
// Register Functions
//------------------------------------------------------------------------------
void GEOSScalarFunctions::RegisterStIntersects(DatabaseInstance &db) {

	ScalarFunctionSet set("ST_Intersects");

	set.AddFunction(ScalarFunction({GeoTypes::GEOMETRY(), GeoTypes::GEOMETRY()}, LogicalType::BOOLEAN,
	                               IntersectsFunction, nullptr, nullptr, nullptr, GEOSFunctionLocalState::Init));

	ExtensionUtil::AddFunctionOverload(db, set);
    DocUtil::AddDocumentation(db, "ST_Intersects", DOC_DESCRIPTION, DOC_EXAMPLE, DOC_TAGS);
}

} // namespace geos

} // namespace spatial
