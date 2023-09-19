#pragma once
#include "spatial/common.hpp"
#include "spatial/core/geometry/geometry_factory.hpp"

namespace spatial {

namespace core {

struct CoreScalarFunctions {
public:
	static void Register(ClientContext &context) {
		RegisterStArea(context);
		RegisterStAsGeoJSON(context);
		RegisterStAsText(context);
		RegisterStAsWKB(context);
		RegisterStAsHEXWKB(context);
		RegisterStCentroid(context);
		RegisterStCollect(context);
		RegisterStCollectionExtract(context);
		RegisterStContains(context);
		RegisterStDimension(context);
		RegisterStDistance(context);
		RegisterStExtent(context);
		RegisterStFlipCoordinates(context);
		RegisterStGeometryType(context);
		RegisterStGeomFromHEXWKB(context);
		RegisterStGeomFromWKB(context);
		RegisterStIntersectsExtent(context);
		RegisterStIsEmpty(context);
		RegisterStLength(context);
		RegisterStMakeLine(context);
		RegisterStNGeometries(context);
		RegisterStNInteriorRings(context);
		RegisterStNPoints(context);
		RegisterStPerimeter(context);
		RegisterStPoint(context);
		RegisterStPointN(context);
		RegisterStRemoveRepeatedPoints(context);
		RegisterStX(context);
		RegisterStXMax(context);
		RegisterStXMin(context);
		RegisterStY(context);
		RegisterStYMax(context);
		RegisterStYMin(context);
	}

private:
	// ST_Area
	static void RegisterStArea(ClientContext &context);

	// ST_AsGeoJSON
	static void RegisterStAsGeoJSON(ClientContext &context);

	// ST_AsText
	static void RegisterStAsText(ClientContext &context);

	// ST_AsHextWKB
	static void RegisterStAsHEXWKB(ClientContext &context);

	// ST_AsWKB
	static void RegisterStAsWKB(ClientContext &context);

	// ST_Centroid
	static void RegisterStCentroid(ClientContext &context);

	// ST_Collect
	static void RegisterStCollect(ClientContext &context);

	// ST_CollectionExtract
	static void RegisterStCollectionExtract(ClientContext &context);

	// ST_Contains
	static void RegisterStContains(ClientContext &context);

	// ST_Dimension
	static void RegisterStDimension(ClientContext &context);

	// ST_Distance
	static void RegisterStDistance(ClientContext &context);

	// ST_Extent
	static void RegisterStExtent(ClientContext &context);

	// ST_FlipCoordinates
	static void RegisterStFlipCoordinates(ClientContext &context);

	// ST_GeometryType
	static void RegisterStGeometryType(ClientContext &context);

	// ST_GeomFromHEXWKB
	static void RegisterStGeomFromHEXWKB(ClientContext &context);

	// ST_GeomFromWKB
	static void RegisterStGeomFromWKB(ClientContext &context);

	// ST_IntersectsExtent (&&)
	static void RegisterStIntersectsExtent(ClientContext &context);

	// ST_IsEmpty
	static void RegisterStIsEmpty(ClientContext &context);

	// ST_Length
	static void RegisterStLength(ClientContext &context);

	// ST_MakeLine
	static void RegisterStMakeLine(ClientContext &context);

	// ST_NGeometries
	static void RegisterStNGeometries(ClientContext &context);

	// ST_NInteriorRings
	static void RegisterStNInteriorRings(ClientContext &context);

	// ST_NPoints
	static void RegisterStNPoints(ClientContext &context);

	// ST_Perimeter
	static void RegisterStPerimeter(ClientContext &context);

	// ST_Point
	static void RegisterStPoint(ClientContext &context);

	// ST_PointN
	static void RegisterStPointN(ClientContext &context);

	// ST_RemoveRepeatedPoints
	static void RegisterStRemoveRepeatedPoints(ClientContext &context);

	// ST_X
	static void RegisterStX(ClientContext &context);

	// ST_XMax
	static void RegisterStXMax(ClientContext &context);

	// ST_XMin
	static void RegisterStXMin(ClientContext &context);

	// ST_Y
	static void RegisterStY(ClientContext &context);

	// ST_YMax
	static void RegisterStYMax(ClientContext &context);

	// ST_YMin
	static void RegisterStYMin(ClientContext &context);
};

} // namespace core

} // namespace spatial