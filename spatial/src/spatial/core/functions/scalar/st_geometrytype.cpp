#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"
#include "spatial/common.hpp"
#include "spatial/core/functions/scalar.hpp"
#include "spatial/core/geometry/geometry.hpp"
#include "spatial/core/functions/common.hpp"
#include "spatial/core/types.hpp"

namespace spatial {

namespace core {

static unique_ptr<FunctionData> GeometryTypeFunctionBind(ClientContext &context, ScalarFunction &bound_function,
                                                         vector<unique_ptr<Expression>> &arguments) {
	// Create an enum type for all geometry types
	// Ensure that these are in the same order as the GeometryType enum
	vector<string> enum_values = {"POINT", "LINESTRING", "POLYGON", "MULTIPOINT", "MULTILINESTRING", "MULTIPOLYGON",
	                                "GEOMETRYCOLLECTION",
	                                // or...
	                                "UNKNOWN"};

	bound_function.return_type = GeoTypes::CreateEnumType("GEOMETRY_TYPE", enum_values);

	return nullptr;
}

//------------------------------------------------------------------------------
// Point2D
//------------------------------------------------------------------------------
static void Point2DTypeFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	result.SetVectorType(VectorType::CONSTANT_VECTOR);
	*ConstantVector::GetData<uint8_t>(result) = static_cast<uint8_t>(GeometryType::POINT);
}

//------------------------------------------------------------------------------
// LineString2D
//------------------------------------------------------------------------------
static void Linestring2DTypeFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	result.SetVectorType(VectorType::CONSTANT_VECTOR);
	*ConstantVector::GetData<uint8_t>(result) = static_cast<uint8_t>(GeometryType::LINESTRING);
}

//------------------------------------------------------------------------------
// Polygon2D
//------------------------------------------------------------------------------
static void Polygon2DTypeFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	result.SetVectorType(VectorType::CONSTANT_VECTOR);
	*ConstantVector::GetData<uint8_t>(result) = static_cast<uint8_t>(GeometryType::POLYGON);
}

//------------------------------------------------------------------------------
// GEOMETRY
//------------------------------------------------------------------------------
static void GeometryTypeFunction(DataChunk &args, ExpressionState &state, Vector &result) {

	auto &lstate = GeometryFunctionLocalState::ResetAndGet(state);

	auto &input = args.data[0];
	auto count = args.size();

	UnaryExecutor::Execute<string_t, uint8_t>(input, result, count, [&](string_t input) {
		auto geom = lstate.factory.Deserialize(input);
		return static_cast<uint8_t>(geom.Type());
	});
}

//------------------------------------------------------------------------------
// Register functions
//------------------------------------------------------------------------------
void CoreScalarFunctions::RegisterStGeometryType(DatabaseInstance &db) {

	ScalarFunctionSet geometry_type_set("ST_GeometryType");
	geometry_type_set.AddFunction(
	    ScalarFunction({GeoTypes::POINT_2D()}, LogicalType::ANY, Point2DTypeFunction, GeometryTypeFunctionBind));
	geometry_type_set.AddFunction(ScalarFunction({GeoTypes::LINESTRING_2D()}, LogicalType::ANY,
	                                             Linestring2DTypeFunction, GeometryTypeFunctionBind));
	geometry_type_set.AddFunction(
	    ScalarFunction({GeoTypes::POLYGON_2D()}, LogicalType::ANY, Polygon2DTypeFunction, GeometryTypeFunctionBind));
	geometry_type_set.AddFunction(ScalarFunction({GeoTypes::GEOMETRY()}, LogicalType::ANY, GeometryTypeFunction,
	                                             GeometryTypeFunctionBind, nullptr, nullptr,
	                                             GeometryFunctionLocalState::Init));

	ExtensionUtil::RegisterFunction(db, geometry_type_set);
}

} // namespace core

} // namespace spatial