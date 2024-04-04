#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"
#include "duckdb/common/vector_operations/generic_executor.hpp"
#include "spatial/common.hpp"
#include "spatial/core/types.hpp"
#include "spatial/gdal/file_handler.hpp"
#include "spatial/gdal/functions/scalar.hpp"
#include "spatial/gdal/raster/raster.hpp"
#include "spatial/gdal/raster/raster_factory.hpp"
#include "spatial/gdal/raster/raster_registry.hpp"

#include "gdal_priv.h"

namespace spatial {

namespace gdal {

//------------------------------------------------------------------------------
// ST_RasterFromFile
//------------------------------------------------------------------------------

static void RasterFromFileFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &context = state.GetContext();

	UnaryExecutor::Execute<string_t, uintptr_t>(args.data[0], result, args.size(), [&](string_t input) {
		auto &ctx_state = GDALClientContextState::GetOrCreate(context);

		auto raw_file_name = input.GetString();
		auto prefixed_file_name = ctx_state.GetPrefix(raw_file_name);

		GDALDataset *dataset = RasterFactory::FromFile(prefixed_file_name);
		if (dataset == nullptr) {
			auto error = Raster::GetLastErrorMsg();
			throw IOException("Could not open file: " + raw_file_name + " (" + error + ")");
		}

		auto &raster_registry = ctx_state.GetRasterRegistry(context);
		raster_registry.RegisterRaster(dataset);

		return CastPointerToValue(dataset);
	});
}

//------------------------------------------------------------------------------
// Register functions
//------------------------------------------------------------------------------

void GdalScalarFunctions::RegisterStRasterFromFile(DatabaseInstance &db) {

	ScalarFunctionSet set("ST_RasterFromFile");
	set.AddFunction(ScalarFunction({LogicalType::VARCHAR}, GeoTypes::RASTER(), RasterFromFileFunction));
	ExtensionUtil::RegisterFunction(db, set);
}

} // namespace gdal

} // namespace spatial
