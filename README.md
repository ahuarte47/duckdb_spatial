# DuckDB Spatial Extension

🚧 WORK IN PROGRESS 🚧

**Table of contents**
- [DuckDB Spatial Extension](#duckdb-spatial-extension)
- [What is this?](#what-is-this)
- [Example Usage](#example-usage)
- [How do I get it?](#how-do-i-get-it)
  - [Through the DuckDB CLI](#through-the-duckdb-cli)
  - [Development builds](#development-builds)
  - [Building from source](#building-from-source)
- [Limitations and Roadmap](#limitations-and-roadmap)
- [Internals and technical details](#internals-and-technical-details)
  - [Multi-tiered Geometry Type System](#multi-tiered-geometry-type-system)
  - [Per-thread Arena Allocation for Geometry Objects](#per-thread-arena-allocation-for-geometry-objects)
  - [Embedded PROJ Database](#embedded-proj-database)
  - [Embedded GDAL based Input/Output Functions](#embedded-gdal-based-inputoutput-functions)
- [Supported Functions](#supported-functions)

# What is this?
This is a prototype of a geospatial extension for DuckDB that adds support for working with spatial data and functions in the form of a `GEOMETRY` type based on the the "Simple Features" geometry model, as well as non-standard specialized columnar DuckDB native geometry types that provide better compression and faster execution in exchange for flexibility.

Please note that this extension is still in a very early stage of development, and the internal storage format for the geometry types may change indiscriminately between commits. We are actively working on it, and we welcome both contributions and feedback. Please see the [function table](#supported-functions) or the [roadmap entries](https://github.com/duckdblabs/duckdb_spatial/labels/roadmap) for the current implementation status.

If you or your organization have any interest in sponsoring development of this extension, or have any particular use cases you'd like to see prioritized or supported, please consider [sponsoring the DuckDB foundation](https://duckdb.org/foundation/) or [contacting DuckDB Labs](https://duckdblabs.com) for commercial support.

# Example Usage
The following is a slightly contrived example of how you can use this extension to read and export multiple geospatial data formats, transform geometries between different coordinate reference systems and work with spatial property and predicate functions.

Let's start by loading the spatial extension and the parquet extension so we can import the NYC taxi ride data in parquet format, and the accompanying taxi zone data from a shapefile, using the spatial `ST_Read` gdal-based table function. We then create a table for the rides and a table for the zones. Note that `ST_Read` produces a table with a `wkb_geometry` column that contains the geometry data encoded as a WKB (Well-Known Binary) blob, which we then convert to the `GEOMETRY` type using the `ST_GeomFromWKB` function.
```sql
LOAD spatial;
LOAD parquet;

CREATE TABLE rides AS SELECT * 
FROM './spatial/test/data/nyc_taxi/yellow_tripdata_2010-01-limit1mil.parquet';

-- Load the NYC taxi zone data from a shapefile using the gdal-based st_read function
CREATE TABLE zones AS SELECT zone, LocationId, borough, ST_GeomFromWKB(wkb_geometry) AS geom 
FROM st_read('./spatial/test/data/nyc_taxi/taxi_zones/taxi_zones.shx');
```
<details>
<summary>
    SELECT * FROM rides LIMIT 10;
</summary>

| vendor_id |   pickup_datetime   |  dropoff_datetime   | passenger_count |   trip_distance    |  pickup_longitude  | pickup_latitude | rate_code | store_and_fwd_flag | dropoff_longitude  | dropoff_latitude | payment_type | fare_amount | surcharge | mta_tax | tip_amount | tolls_amount | total_amount |
|-----------|---------------------|---------------------|-----------------|--------------------|--------------------|-----------------|-----------|--------------------|--------------------|------------------|--------------|-------------|-----------|---------|------------|--------------|--------------|
| VTS       | 2010-01-01 00:00:17 | 2010-01-01 00:00:17 | 3               | 0.0                | -73.87105699999998 | 40.773522       | 1         |                    | -73.871048         | 40.773545        | CAS          | 45.0        | 0.0       | 0.5     | 0.0        | 0.0          | 45.5         |
| VTS       | 2010-01-01 00:00:20 | 2010-01-01 00:00:20 | 1               | 0.05               | -73.97512999999998 | 40.789973       | 1         |                    | -73.97498799999998 | 40.790598        | CAS          | 2.5         | 0.5       | 0.5     | 0.0        | 0.0          | 3.5          |
| CMT       | 2010-01-01 00:00:23 | 2010-01-01 00:00:25 | 1               | 0.0                | -73.999431         | 40.71216        | 1         | 0                  | -73.99915799999998 | 40.712421        | No           | 2.5         | 0.5       | 0.5     | 0.0        | 0.0          | 3.5          |
| CMT       | 2010-01-01 00:00:33 | 2010-01-01 00:00:55 | 1               | 0.0                | -73.97721699999998 | 40.749633       | 1         | 0                  | -73.97732899999998 | 40.749629        | Cas          | 2.5         | 0.5       | 0.5     | 0.0        | 0.0          | 3.5          |
| VTS       | 2010-01-01 00:01:00 | 2010-01-01 00:01:00 | 1               | 0.0                | -73.942313         | 40.784332       | 1         |                    | -73.942313         | 40.784332        | Cre          | 10.0        | 0.0       | 0.5     | 2.0        | 0.0          | 12.5         |
| VTS       | 2010-01-01 00:01:06 | 2010-01-01 00:01:06 | 2               | 0.38               | -73.97463          | 40.756687       | 1         |                    | -73.979872         | 40.759143        | CAS          | 3.7         | 0.5       | 0.5     | 0.0        | 0.0          | 4.7          |
| VTS       | 2010-01-01 00:01:07 | 2010-01-01 00:01:07 | 2               | 0.23               | -73.987358         | 40.718475       | 1         |                    | -73.98518          | 40.720468        | CAS          | 2.9         | 0.5       | 0.5     | 0.0        | 0.0          | 3.9          |
| CMT       | 2010-01-01 00:00:02 | 2010-01-01 00:01:08 | 1               | 0.1                | -73.992807         | 40.741418       | 1         | 0                  | -73.995799         | 40.742596        | No           | 2.9         | 0.5       | 0.5     | 0.0        | 0.0          | 3.9          |
| VTS       | 2010-01-01 00:01:23 | 2010-01-01 00:01:23 | 1               | 0.6099999999999999 | -73.98003799999998 | 40.74306        | 1         |                    | -73.974862         | 40.750387        | CAS          | 3.7         | 0.5       | 0.5     | 0.0        | 0.0          | 4.7          |
| VTS       | 2010-01-01 00:01:34 | 2010-01-01 00:01:34 | 1               | 0.02               | -73.954122         | 40.801173       | 1         |                    | -73.95431499999998 | 40.800897        | CAS          | 45.0        | 0.0       | 0.5     | 0.0        | 0.0          | 45.5         |

</details>

<details>
<summary>
    SELECT * FROM zones LIMIT 10;
</summary>

|          zone           | LocationID |    borough    |  geom              |
|-------------------------|------------|---------------|--------------------|
| Newark Airport          | 1          | EWR           | POLYGON (...)      |
| Jamaica Bay             | 2          | Queens        | MULTIPOLYGON (...) |
| Allerton/Pelham Gardens | 3          | Bronx         | POLYGON (...)      |
| Alphabet City           | 4          | Manhattan     | POLYGON (...)      |
| Arden Heights           | 5          | Staten Island | POLYGON (...)      |
| Arrochar/Fort Wadsworth | 6          | Staten Island | POLYGON (...)      |
| Astoria                 | 7          | Queens        | POLYGON (...)      |
| Astoria Park            | 8          | Queens        | POLYGON (...)      |
| Auburndale              | 9          | Queens        | POLYGON (...)      |
| Baisley Park            | 10         | Queens        | POLYGON (...)      |

</details>

Let's compare the trip distance to the linear distance between the pickup and dropoff points to figure out how efficient the taxi drivers are (or how dirty the data is, since some diffs seem to be negative). We transform the coordinates from WGS84 (EPSG:4326) (lat/lon) to the NAD83 / New York Long Island ftUS (ESRI:102718) projection and calculate the distance using the `ST_Distance` function, which in this case gives the distance in feet, which we then convert to miles (5280 ft/mile). Trips that are smaller than the aerial distance are likely to be erroneous, so we use this query to filter out some bad data. Although this is not entirely accurate since the distance we use does not take into account the curvature of the earth, but it is a good enough approximation for our purposes.
```sql
CREATE TABLE cleaned_rides AS SELECT 
    st_point(pickup_latitude, pickup_longitude) as pickup_point,
    st_point(dropoff_latitude, dropoff_longitude) as dropoff_point,
    dropoff_datetime::TIMESTAMP - pickup_datetime::TIMESTAMP as time,
    trip_distance,
    st_distance(
        st_transform(pickup_point, 'EPSG:4326', 'ESRI:102718'), 
        st_transform(dropoff_point, 'EPSG:4326', 'ESRI:102718')) / 5280 as aerial_distance, 
    trip_distance - aerial_distance as diff 
FROM rides 
WHERE diff > 0
ORDER BY diff DESC;
```
<details>
<summary>
    SELECT * FROM cleaned_rides LIMIT 10;
</summary>

|             pickup_point             |            dropoff_point             |   time   | trip_distance |   aerial_distance    |        diff        |
|--------------------------------------|--------------------------------------|----------|---------------|----------------------|--------------------|
| POINT (40.758149 -73.963267)         | POINT (40.743807 -73.915763)         | 01:49:25 | 47.4          | 2.6820365663951677   | 44.71796343360483  |
| POINT (40.764592 -73.971798)         | POINT (40.743878 -73.991015)         | 01:09:29 | 45.9          | 1.7492118606943174   | 44.15078813930568  |
| POINT (40.733306 -73.987289)         | POINT (40.758895 -73.987341)         | 02:15:53 | 45.2          | 1.7657013363262366   | 43.434298663673765 |
| POINT (40.755965 -73.973138)         | POINT (40.756137 -73.973535)         | 02:48:19 | 41.9          | 0.02397481410159387  | 41.876025185898406 |
| POINT (40.645337 -73.77656899999998) | POINT (40.645389 -73.77632699999998) | 00:50:09 | 41.4          | 0.013215531558232116 | 41.38678446844177  |
| POINT (40.743983 -73.986063)         | POINT (40.759483 -73.985377)         | 00:00:18 | 41.9          | 1.070141742954722    | 40.829858257045274 |
| POINT (40.782197 -73.98247)          | POINT (40.713868 -74.03883)          | 02:14:00 | 45.05         | 5.565739906339228    | 39.484260093660765 |
| POINT (40.662826 -73.78962799999998) | POINT (40.656764 -73.794222)         | 02:19:32 | 39.8          | 0.4829476946393737   | 39.317052305360626 |
| POINT (40.644837 -73.781722)         | POINT (40.646732 -73.801497)         | 01:29:00 | 39.8          | 1.0475276883275062   | 38.75247231167249  |
| POINT (40.754573 -73.98841199999998) | POINT (40.763428 -73.968002)         | 01:31:00 | 39.74         | 1.2329411412165936   | 38.50705885878341  |
</details>

Now lets join the taxi rides with the taxi zones to get the start and end zone for each ride using the `ST_Within` function to check if a point is within a polygon. Again we need to transform the coordinates from WGS84 to the NAD83 since the taxi zone data also use that projection.
```sql
-- Since we dont have spatial indexes yet, use a smaller dataset for the following example.
DELETE FROM cleaned_rides WHERE rowid > 5000;

CREATE TABLE joined AS 
SELECT 
    pickup_point,
    dropoff_point,
    start_zone.zone as start_zone,
    end_zone.zone as end_zone, 
    trip_distance,
    time,
FROM cleaned_rides 
JOIN zones as start_zone 
ON ST_Within(st_transform(pickup_point, 'EPSG:4326', 'ESRI:102718'), start_zone.geom) 
JOIN zones as end_zone 
ON ST_Within(st_transform(dropoff_point, 'EPSG:4326', 'ESRI:102718'), end_zone.geom);
```
<details>
<summary>
    SELECT * FROM joined USING SAMPLE 10 ROWS;
</summary>

|             pickup_point             |            dropoff_point             |        start_zone        |           end_zone            | trip_distance |   time   |
|--------------------------------------|--------------------------------------|--------------------------|-------------------------------|---------------|----------|
| POINT (40.722223 -73.98385299999998) | POINT (40.715507 -73.992438)         | East Village             | Lower East Side               | 10.3          | 00:19:16 |
| POINT (40.648687 -73.783522)         | POINT (40.649567 -74.005812)         | JFK Airport              | Sunset Park West              | 23.57         | 00:28:00 |
| POINT (40.761603 -73.96661299999998) | POINT (40.760232 -73.96344499999998) | Upper East Side South    | Sutton Place/Turtle Bay North | 17.6          | 00:27:05 |
| POINT (40.697212 -73.937495)         | POINT (40.652377 -73.93983299999998) | Stuyvesant Heights       | East Flatbush/Farragut        | 13.55         | 00:24:00 |
| POINT (40.721462 -73.993583)         | POINT (40.774205 -73.90441699999998) | Lower East Side          | Steinway                      | 28.75         | 01:03:00 |
| POINT (40.716955 -74.004328)         | POINT (40.754688 -73.991612)         | TriBeCa/Civic Center     | Garment District              | 18.4          | 00:46:12 |
| POINT (40.740052 -73.994918)         | POINT (40.75439 -73.98587499999998)  | Flatiron                 | Garment District              | 24.2          | 00:35:25 |
| POINT (40.763017 -73.95949199999998) | POINT (40.763615 -73.959182)         | Lenox Hill East          | Lenox Hill West               | 18.4          | 00:33:46 |
| POINT (40.865663 -73.927458)         | POINT (40.86537 -73.927352)          | Washington Heights North | Washington Heights North      | 10.47         | 00:27:00 |
| POINT (40.738408 -73.980345)         | POINT (40.696038 -73.955493)         | Gramercy                 | Bedford                       | 16.4          | 00:21:47 |
</details>

We can export the joined table to a GeoJSONSeq file using the GDAL copy function, passing in a GDAL layer creation option. 
Since GeoJSON only supports a single geometry per feature, we can use the `ST_MakeLine` function to combine the pickup and dropoff points into a single line geometry. The default coordinate reference system for GeoJSON is WGS84, but the coordinates are expected to be in longitude/latitude, so we need to flip the geometry using the `ST_FlipCoordinates` function.

```sql
COPY (
    SELECT 
        ST_AsWKB(ST_FlipCoordinates(ST_MakeLine(pickup_point, dropoff_point))) as wkb_geometry,
        start_zone,
        end_zone,
        time::VARCHAR as trip_time 
    FROM joined) 
TO 'joined.geojsonseq' 
WITH (FORMAT GDAL, DRIVER 'GeoJSONSeq', LAYER_CREATION_OPTIONS 'WRITE_BBOX=YES');
```
<details>
<summary>
    head -n 10 joined.geojsonseq
</summary>

```json
{ "type": "Feature", "properties": { "start_zone": "JFK Airport", "end_zone": "Park Slope", "trip_time": "00:52:00" }, "geometry": { "type": "LineString", "coordinates": [ [ -73.789923, 40.643515 ], [ -73.97608, 40.680395 ] ] } }
{ "type": "Feature", "properties": { "start_zone": "JFK Airport", "end_zone": "Park Slope", "trip_time": "00:35:00" }, "geometry": { "type": "LineString", "coordinates": [ [ -73.776445, 40.645422 ], [ -73.98427, 40.670782 ] ] } }
{ "type": "Feature", "properties": { "start_zone": "JFK Airport", "end_zone": "Park Slope", "trip_time": "00:45:42" }, "geometry": { "type": "LineString", "coordinates": [ [ -73.776878, 40.645065 ], [ -73.992153, 40.662571 ] ] } }
{ "type": "Feature", "properties": { "start_zone": "JFK Airport", "end_zone": "Park Slope", "trip_time": "00:36:00" }, "geometry": { "type": "LineString", "coordinates": [ [ -73.788028, 40.641508 ], [ -73.97584, 40.670927 ] ] } }
{ "type": "Feature", "properties": { "start_zone": "JFK Airport", "end_zone": "Park Slope", "trip_time": "00:47:58" }, "geometry": { "type": "LineString", "coordinates": [ [ -73.781855, 40.644749 ], [ -73.980129, 40.663663 ] ] } }
{ "type": "Feature", "properties": { "start_zone": "JFK Airport", "end_zone": "Park Slope", "trip_time": "00:32:10" }, "geometry": { "type": "LineString", "coordinates": [ [ -73.787494, 40.641559 ], [ -73.974694, 40.673479 ] ] } }
{ "type": "Feature", "properties": { "start_zone": "JFK Airport", "end_zone": "Park Slope", "trip_time": "00:36:59" }, "geometry": { "type": "LineString", "coordinates": [ [ -73.790138, 40.643342 ], [ -73.982721, 40.662379 ] ] } }
{ "type": "Feature", "properties": { "start_zone": "JFK Airport", "end_zone": "Park Slope", "trip_time": "00:32:00" }, "geometry": { "type": "LineString", "coordinates": [ [ -73.786952, 40.641248 ], [ -73.97421, 40.676237 ] ] } }
{ "type": "Feature", "properties": { "start_zone": "JFK Airport", "end_zone": "Park Slope", "trip_time": "00:33:21" }, "geometry": { "type": "LineString", "coordinates": [ [ -73.783892, 40.648514 ], [ -73.979283, 40.669721 ] ] } }
{ "type": "Feature", "properties": { "start_zone": "JFK Airport", "end_zone": "Park Slope", "trip_time": "00:35:45" }, "geometry": { "type": "LineString", "coordinates": [ [ -73.776643, 40.645272 ], [ -73.978873, 40.66723 ] ] } }
```
</details>


# How do I get it?

## Through the DuckDB CLI
You can install the extension for DuckDB v0.7.1 through the DuckDB CLI like you would do for other first party extensions. Simply execute: ```INSTALL spatial; LOAD spatial```!

## Development builds
You can also grab the lastest builds directly from the CI runs or the release page here on GitHub and install manually.

Once you have downloaded the extension for your platform, you need to:
- Unzip the archive
- Start duckdb with the `-unsigned` flag to allow loading unsigned extensions. (This won't be neccessary in the future)
- Run `INSTALL 'absolute/or/relative/path/to/the/unzipped/extension';`
- The extension is now installed, you can now load it with `LOAD spatial;` whenever you want to use it.

You can also build the extension yourself following the instructions below.

## Building from source
This extension is based on the [DuckDB extension template](https://github.com/duckdb/extension-template).

**Dependencies**

You need a recent version of CMake (3.20) and a C++11 compatible compiler.
If you're cross-compiling, you need a host sqlite3 executable in your path, otherwise the build should create and use its own sqlite3 executable. (This is required for creating the PROJ database).
You also need OpenSSL on your system. On ubuntu you can install it with `sudo apt install libssl-dev`, on macOS you can install it with `brew install openssl`. Note that brew installs openssl in a non-standard location, so you may need to set a `OPENSSL_ROOT_DIR=$(brew --prefix openssl)` environment variable when building.

We bundle all the other required dependencies in the `third_party` directory, which should be automatically built and statically linked into the extension. This may take some time the first time you build, but subsequent builds should be much faster.

We also highly recommend that you install [Ninja](https://ninja-build.org) which you can select when building by setting the `GEN=ninja` environment variable.

```
git clone --recurse-submodules https://github.com/duckdblabs/duckdb_spatial
cd duckdb_spatial
make debug
```
You can then invoke the built DuckDB (with the extension statically linked)
```
./build/debug/duckdb
```

Please see the Makefile for more options, or the extension template documentation for more details.

# Limitations and Roadmap

The main limitations of this extension currently are:
- No support for higher-dimensional geometries (XYZ, XYZM, XYM)
- No support for spherical geometry (e.g. lat/lon coordinates)
- No support for spatial indexing.

These are all things that we want to address eventually, have a look at the open issues and [roadmap entries](https://github.com/duckdblabs/duckdb_spatial/labels/roadmap) for more details. Please feel free to also open an issue if you have a specific use case that you would like to see supported.

# Internals and technical details

## Multi-tiered Geometry Type System
This extension implements 5 different geometry types. Like almost all geospatial databases we include a `GEOMETRY` type that (at least strives) to follow the Simple Features geometry model. This includes support for the standard subtypes, such as `POINT`, `LINESTRING`, `POLYGON`, `MULTIPOINT`, `MULTILINESTRING`, `MULTIPOLYGON`, `GEOMETRYCOLLECTION` that we all know and love, internally represented in a row-wise fashion on top of DuckDB `BLOB`s. The internal binary format is very similar to the one used by PostGIS - basically `double` aligned WKB, and we may eventually look into enforcing the format to be properly compatible with PostGIS (which may be useful for the PostGIS scanner extension). Most functions that are implemented for this type uses the [GEOS library](https://github.com/libgeos/geos), which is a battle-tested C++ port of the famous `JTS` library, to perform the actual operations on the geometries.

While having a flexible and dynamic `GEOMETRY` type is great to have, it is comparatively rare to work with columns containing mixed-geometries after the initial import and cleanup step. In fact, in most OLAP use cases you will probably only have a single geometry type in a table, and in those cases you're paying the performance cost to de/serialize and branch on the internal geometry format unneccessarily, i.e. you're paying for flexibility you're not using. For those cases we implement a set of non-standard DuckDB "native" geometry types, `POINT_2D`, `LINESTRING_2D`, `POLYGON_2D`, and `BOX_2D`. These types are built on DuckDBs `STRUCT` and `LIST` types, and are stored in a columnar fashion with the coordinate dimensions stored in separate "vectors". This makes it possible to leverage DuckDB's per-column statistics, compress much more efficiently and perform spatial operations on these geometries without having to de/serialize them first. Storing the coordinate dimensions into separate vectors also allows casting and converting between geometries with multiple different dimensions basically for free. And if you truly need to mix a couple of different geometry types, you can always use a DuckDB [UNION type](https://duckdb.org/docs/sql/data_types/union).

For now only a small amount of spatial functions are overloaded for these native types, but since they can be implicitly cast to `GEOMETRY` you can always use any of the functions that are implemented for `GEOMETRY` on them as well in the meantime while we work on adding more (although with a de/serialization penalty).

This extension also includes a `WKB_BLOB` type as an alias for `BLOB` that is used to indicate that the blob contains valid WKB encoded geometry.

## Per-thread Arena Allocation for Geometry Objects
When materializing the `GEOMETRY` type objects from the internal binary format we use per-thread arena allocation backed by DuckDB's buffer manager to amortize the contention and performance cost of performing lots of small heap allocations and frees, which allows us to utilizes DuckDB's multi-threaded vectorized out-of-core execution fully. While most spatial functions are implemented by wrapping `GEOS`, which requires an extra copy/allocation step anyway, the plan is to incrementally implementat our own versions of the simpler functions that can operate directly on our own `GEOMETRY` representation in order to greatly accelerate geospatial processing.

## Embedded PROJ Database
[PROJ](https://proj.org/#) is a generic coordinate transformation library that transforms geospatial coordinates from one projected coordinate reference system (CRS) to another. This extension experiments with including an embedded version of the PROJ database inside the extension binary itself so that you don't have to worry about installing the PROJ library separately. This also opens up the possibility to use this functionality in WASM.

## Embedded GDAL based Input/Output Functions
[GDAL](https://github.com/OSGeo/gdal) is a translator library for raster and vector geospatial data formats. This extension includes and exposes a subset of the GDAL vector drivers through the `ST_Read` and `COPY ... TO ... WITH (FORMAT GDAL)` table and copy functions respectively to read and write geometry data from and to a variety of file formats as if they were DuckDB tables. We currently support the over 50 GDAL formats - check for yourself by running 
<details>
<summary>
`SELECT * FROM st_drivers();`!
</summary>

|   short_name   |                      long_name                       | can_create | can_copy | can_open |                      help_url                      |
|----------------|------------------------------------------------------|------------|----------|----------|----------------------------------------------------|
| ESRI Shapefile | ESRI Shapefile                                       | true       | false    | true     | https://gdal.org/drivers/vector/shapefile.html     |
| MapInfo File   | MapInfo File                                         | true       | false    | true     | https://gdal.org/drivers/vector/mitab.html         |
| UK .NTF        | UK .NTF                                              | false      | false    | true     | https://gdal.org/drivers/vector/ntf.html           |
| LVBAG          | Kadaster LV BAG Extract 2.0                          | false      | false    | true     | https://gdal.org/drivers/vector/lvbag.html         |
| S57            | IHO S-57 (ENC)                                       | true       | false    | true     | https://gdal.org/drivers/vector/s57.html           |
| DGN            | Microstation DGN                                     | true       | false    | true     | https://gdal.org/drivers/vector/dgn.html           |
| OGR_VRT        | VRT - Virtual Datasource                             | false      | false    | true     | https://gdal.org/drivers/vector/vrt.html           |
| Memory         | Memory                                               | true       | false    | true     |                                                    |
| CSV            | Comma Separated Value (.csv)                         | true       | false    | true     | https://gdal.org/drivers/vector/csv.html           |
| GML            | Geography Markup Language (GML)                      | true       | false    | true     | https://gdal.org/drivers/vector/gml.html           |
| GPX            | GPX                                                  | true       | false    | true     | https://gdal.org/drivers/vector/gpx.html           |
| KML            | Keyhole Markup Language (KML)                        | true       | false    | true     | https://gdal.org/drivers/vector/kml.html           |
| GeoJSON        | GeoJSON                                              | true       | false    | true     | https://gdal.org/drivers/vector/geojson.html       |
| GeoJSONSeq     | GeoJSON Sequence                                     | true       | false    | true     | https://gdal.org/drivers/vector/geojsonseq.html    |
| ESRIJSON       | ESRIJSON                                             | false      | false    | true     | https://gdal.org/drivers/vector/esrijson.html      |
| TopoJSON       | TopoJSON                                             | false      | false    | true     | https://gdal.org/drivers/vector/topojson.html      |
| OGR_GMT        | GMT ASCII Vectors (.gmt)                             | true       | false    | true     | https://gdal.org/drivers/vector/gmt.html           |
| GPKG           | GeoPackage                                           | true       | true     | true     | https://gdal.org/drivers/vector/gpkg.html          |
| SQLite         | SQLite / Spatialite                                  | true       | false    | true     | https://gdal.org/drivers/vector/sqlite.html        |
| WAsP           | WAsP .map format                                     | true       | false    | true     | https://gdal.org/drivers/vector/wasp.html          |
| OpenFileGDB    | ESRI FileGDB                                         | true       | false    | true     | https://gdal.org/drivers/vector/openfilegdb.html   |
| DXF            | AutoCAD DXF                                          | true       | false    | true     | https://gdal.org/drivers/vector/dxf.html           |
| CAD            | AutoCAD Driver                                       | false      | false    | true     | https://gdal.org/drivers/vector/cad.html           |
| FlatGeobuf     | FlatGeobuf                                           | true       | false    | true     | https://gdal.org/drivers/vector/flatgeobuf.html    |
| Geoconcept     | Geoconcept                                           | true       | false    | true     |                                                    |
| GeoRSS         | GeoRSS                                               | true       | false    | true     | https://gdal.org/drivers/vector/georss.html        |
| VFK            | Czech Cadastral Exchange Data Format                 | false      | false    | true     | https://gdal.org/drivers/vector/vfk.html           |
| PGDUMP         | PostgreSQL SQL dump                                  | true       | false    | false    | https://gdal.org/drivers/vector/pgdump.html        |
| OSM            | OpenStreetMap XML and PBF                            | false      | false    | true     | https://gdal.org/drivers/vector/osm.html           |
| GPSBabel       | GPSBabel                                             | true       | false    | true     | https://gdal.org/drivers/vector/gpsbabel.html      |
| WFS            | OGC WFS (Web Feature Service)                        | false      | false    | true     | https://gdal.org/drivers/vector/wfs.html           |
| OAPIF          | OGC API - Features                                   | false      | false    | true     | https://gdal.org/drivers/vector/oapif.html         |
| EDIGEO         | French EDIGEO exchange format                        | false      | false    | true     | https://gdal.org/drivers/vector/edigeo.html        |
| SVG            | Scalable Vector Graphics                             | false      | false    | true     | https://gdal.org/drivers/vector/svg.html           |
| ODS            | Open Document/ LibreOffice / OpenOffice Spreadsheet  | true       | false    | true     | https://gdal.org/drivers/vector/ods.html           |
| XLSX           | MS Office Open XML spreadsheet                       | true       | false    | true     | https://gdal.org/drivers/vector/xlsx.html          |
| Elasticsearch  | Elastic Search                                       | true       | false    | true     | https://gdal.org/drivers/vector/elasticsearch.html |
| Carto          | Carto                                                | true       | false    | true     | https://gdal.org/drivers/vector/carto.html         |
| AmigoCloud     | AmigoCloud                                           | true       | false    | true     | https://gdal.org/drivers/vector/amigocloud.html    |
| SXF            | Storage and eXchange Format                          | false      | false    | true     | https://gdal.org/drivers/vector/sxf.html           |
| Selafin        | Selafin                                              | true       | false    | true     | https://gdal.org/drivers/vector/selafin.html       |
| JML            | OpenJUMP JML                                         | true       | false    | true     | https://gdal.org/drivers/vector/jml.html           |
| PLSCENES       | Planet Labs Scenes API                               | false      | false    | true     | https://gdal.org/drivers/vector/plscenes.html      |
| CSW            | OGC CSW (Catalog  Service for the Web)               | false      | false    | true     | https://gdal.org/drivers/vector/csw.html           |
| VDV            | VDV-451/VDV-452/INTREST Data Format                  | true       | false    | true     | https://gdal.org/drivers/vector/vdv.html           |
| MVT            | Mapbox Vector Tiles                                  | true       | false    | true     | https://gdal.org/drivers/vector/mvt.html           |
| NGW            | NextGIS Web                                          | true       | true     | true     | https://gdal.org/drivers/vector/ngw.html           |
| MapML          | MapML                                                | true       | false    | true     | https://gdal.org/drivers/vector/mapml.html         |
| TIGER          | U.S. Census TIGER/Line                               | false      | false    | true     | https://gdal.org/drivers/vector/tiger.html         |
| AVCBin         | Arc/Info Binary Coverage                             | false      | false    | true     | https://gdal.org/drivers/vector/avcbin.html        |
| AVCE00         | Arc/Info E00 (ASCII) Coverage                        | false      | false    | true     | https://gdal.org/drivers/vector/avce00.html        |

</details>
Note that far from all of these formats have been tested properly, if you run into any issues please first [consult the GDAL docs](https://gdal.org/drivers/vector/index.html), or open an issue here on GitHub.


`ST_Read` also supports limited support for predicate pushdown and spatial filtering (if the underlying GDAL driver supports it), but column pruning (projection pushdown) while technically feasible is not yet implemented. 
`ST_Read` also allows using GDAL's virtual filesystem abstractions to read data from remote sources such as S3, or from compressed archives such as zip files. 

**Note**: This functionality does not make full use of parallelism due to GDAL not being thread-safe, so you should expect this to be slower than using e.g. the DuckDB Parquet extension to read the same GeoParquet or DuckDBs native csv reader to read csv files. Once we implement support for reading more vector formats natively through this extension (e.g. GeoJSON, GeoBuf, ShapeFile) we will probably split this entire GDAL part into a separate extension.


# Supported Functions

🧭 - GEOS - functions that are implemented using the [GEOS](https://libgeos.org/) library

🦆 - DuckDB - functions that are implemented natively in this extension that are capable of operating directly on the DuckDB types

🔄 - CAST(GEOMETRY) - functions that are supported by implicitly casting to `GEOMETRY` and then using the `GEOMETRY` implementation

We are actively working on implementing more functions, and will update this table as we go.
Again, please feel free to open an issue if there is a particular function you would like to see implemented. Contributions are also welcome!


| Scalar functions            | GEOMETRY | POINT_2D   | LINESTRING_2D | POLYGON_2D | BOX_2D          |
| --------------------------- | -------- | ---------- | ------------- | ---------- | --------------- |
| ST_Point                    | 🦆        | 🦆        |               |            |                 |
| ST_Area                     | 🦆        | 🦆        | 🦆            | 🦆         | 🦆              |
| ST_AsGeoJSON                | 🦆        | 🦆        | 🦆            | 🦆.        | 🦆              |
| ST_AsHEXWKB                 | 🦆        | 🦆        | 🦆            | 🦆         | 🦆              |
| ST_AsText                   | 🦆        | 🦆        | 🦆            | 🦆         | 🦆              | 
| ST_AsWKB                    | 🦆        | 🦆        | 🦆            | 🦆         | 🦆              |
| ST_Boundary                 | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_Buffer                   | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_Centroid                 | 🧭        | 🦆        | 🦆            | 🦆         | 🦆              |
| ST_Collect                  | 🦆        | 🦆        | 🦆            | 🦆         | 🦆              |
| ST_CollectionExtract        | 🦆        |           |               |            |                 |
| ST_Contains                 | 🧭        | 🔄        | 🔄            | 🦆 or 🔄   | 🔄 (as POLYGON) |
| ST_ContainsProperly         | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_ConvexHull               | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_CoveredBy                | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_Covers                   | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_Crosses                  | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_Difference               | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_Dimension                | 🦆        |           |               |            |                 |
| ST_Disjoint                 | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_Distance                 | 🧭        | 🦆 or 🔄  | 🦆 or 🔄      | 🔄         | 🔄 (as POLYGON) |
| ST_DWithin                  | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_Envelope                 | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_Equals                   | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_FlipCoordinates          | 🦆        | 🦆        | 🦆            | 🦆         | 🦆              |
| ST_GeomFromText             | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_GeomFromWKB              | 🦆        | 🦆        | 🦆            | 🦆         | 🔄 (as POLYGON) |
| ST_GeometryType             | 🦆        | 🦆        | 🦆            | 🦆         | 🔄 (as POLYGON) |
| ST_MakeLine                 | 🦆        |           | 🦆            |            |                 |
| ST_Perimeter                | 🦆        | 🦆        | 🦆            | 🦆         | 🦆              |
| ST_Intersection             | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_Intersects               | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_IsClosed                 | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_IsEmpty                  | 🦆        | 🦆        | 🦆            | 🦆         | 🔄 (as POLYGON) |
| ST_IsRing                   | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_IsSimple                 | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_IsValid                  | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_Length                   | 🦆        | 🦆        | 🦆            | 🦆         | 🔄 (as POLYGON) |
| ST_Normalize                | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_NumPoints/ST_NPoints     | 🦆        | 🦆        | 🦆            | 🦆         | 🦆              |
| ST_Overlaps                 | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_SimplifyPreserveTopology | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_Simplify                 | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_Touches                  | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_Union                    | 🧭        | 🔄        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_Within                   | 🧭        | 🦆 or 🔄  | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_X                        | 🧭        | 🦆        | 🔄            | 🔄         | 🔄 (as POLYGON) |
| ST_Y                        | 🧭        | 🦆        | 🔄            | 🔄         | 🔄 (as POLYGON) |


```
