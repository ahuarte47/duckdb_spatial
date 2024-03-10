#pragma once

#include "spatial/common.hpp"
#include "spatial/core/types.hpp"
#include <cmath>

namespace spatial {

namespace core {

struct VertexXY {
	static const constexpr bool IS_VERTEX = true;
	static const constexpr bool HAS_Z = false;
	static const constexpr bool HAS_M = false;

	double x;
	double y;
};

struct VertexXYZ {
	static const constexpr bool IS_VERTEX = true;
	static const constexpr bool HAS_Z = true;
	static const constexpr bool HAS_M = false;

	double x;
	double y;
	double z;
};

struct VertexXYM {
	static const constexpr bool IS_VERTEX = true;
	static const constexpr bool HAS_Z = false;
	static const constexpr bool HAS_M = true;

	double x;
	double y;
	double m;
};

struct VertexXYZM {
	static const constexpr bool IS_VERTEX = true;
	static const constexpr bool HAS_Z = true;
	static const constexpr bool HAS_M = true;

	double x;
	double y;
	double z;
	double m;
};

// A vertex array represents a copy-on-write array of potentially non-owned vertex data
class VertexArray {
private:
	class Properties {
	private:
		uint32_t data;
		uint32_t vertex_size;

	public:
		Properties() : data(0), vertex_size(sizeof(double) * 2) {
		}
		Properties(bool has_z, bool has_m)
		    : data((has_z ? 1 : 0) | (has_m ? 2 : 0)),
		      vertex_size(sizeof(double) * (2 + (has_z ? 1 : 0) + (has_m ? 1 : 0))) {
		}
		bool HasZ() const {
			return (data & 1) != 0;
		}
		bool HasM() const {
			return (data & 2) != 0;
		}
		void SetZ(bool has_z) {
			data = (data & 2) | (has_z ? 1 : 0);
			vertex_size = sizeof(double) * (2 + (has_z ? 1 : 0) + (HasM() ? 1 : 0));
		}
		void SetM(bool has_m) {
			data = (data & 1) | (has_m ? 2 : 0);
			vertex_size = sizeof(double) * (2 + (HasZ() ? 1 : 0) + (has_m ? 1 : 0));
		}
		uint32_t VertexSize() const {
			return vertex_size;
		}
	};

private:
	reference<Allocator> alloc;
	data_ptr_t vertex_data;
	uint32_t vertex_count;
	uint32_t owned_capacity;
	Properties properties;

public:
	// Create a non-owning reference to the vertex data
	VertexArray(Allocator &alloc_p, data_ptr_t vertex_data_p, uint32_t vertex_count_p, bool has_z, bool has_m)
	    : alloc(alloc_p), vertex_data(vertex_data_p), vertex_count(vertex_count_p), owned_capacity(0),
	      properties(has_z, has_m) {
	}

	// Create an owning array with the given capacity
	VertexArray(Allocator &alloc_p, uint32_t owned_capacity_p, bool has_z, bool has_m)
	    : alloc(alloc_p),
	      vertex_data(owned_capacity_p == 0
	                      ? nullptr
	                      : alloc.get().AllocateData(owned_capacity_p *
	                                                 (sizeof(double) * (2 + (has_z ? 1 : 0) + (has_m ? 1 : 0))))),
	      vertex_count(0), owned_capacity(owned_capacity_p), properties(has_z, has_m) {
	}

	// Create an empty non-owning array
	VertexArray(Allocator &alloc_p, bool has_z, bool has_m)
	    : alloc(alloc_p), vertex_data(nullptr), vertex_count(0), owned_capacity(0), properties(has_z, has_m) {
	}

	// Create an empty non-owning array
	static VertexArray CreateEmpty(Allocator &alloc_p, bool has_z, bool has_m) {
		return VertexArray {alloc_p, nullptr, 0, has_z, has_m};
	}

	~VertexArray() {
		if (IsOwning() && vertex_data != nullptr) {
			alloc.get().FreeData(vertex_data, owned_capacity * properties.VertexSize());
		}
	}

	//-------------------------------------------------------------------------
	// Copy (reference the same data but do not own it)
	//-------------------------------------------------------------------------

	VertexArray(const VertexArray &other)
	    : alloc(other.alloc), vertex_data(other.vertex_data), vertex_count(other.vertex_count), owned_capacity(0),
	      properties(other.properties) {
	}

	VertexArray &operator=(const VertexArray &other) {
		if (IsOwning() && vertex_data != nullptr) {
			alloc.get().FreeData(vertex_data, owned_capacity * properties.VertexSize());
		}
		alloc = other.alloc;
		vertex_data = other.vertex_data;
		vertex_count = other.vertex_count;
		owned_capacity = 0;
		properties = other.properties;
		return *this;
	}

	//-------------------------------------------------------------------------
	// Move (take ownership of the data)
	//-------------------------------------------------------------------------

	VertexArray(VertexArray &&other) noexcept
	    : alloc(other.alloc), vertex_data(other.vertex_data), vertex_count(other.vertex_count),
	      owned_capacity(other.owned_capacity), properties(other.properties) {
		other.vertex_data = nullptr;
		other.vertex_count = 0;
		other.owned_capacity = 0;
	}

	VertexArray &operator=(VertexArray &&other) noexcept {
		if (IsOwning() && vertex_data != nullptr) {
			alloc.get().FreeData(vertex_data, owned_capacity * properties.VertexSize());
		}
		alloc = other.alloc;
		vertex_data = other.vertex_data;
		vertex_count = other.vertex_count;
		owned_capacity = other.owned_capacity;
		other.vertex_data = nullptr;
		other.vertex_count = 0;
		other.owned_capacity = 0;
		return *this;
	}

	//-------------------------------------------------------------------------
	// CoW
	//-------------------------------------------------------------------------

	bool IsOwning() const {
		return owned_capacity > 0;
	}

	VertexArray &MakeOwning() {
		if (IsOwning()) {
			return *this;
		}

		// Ensure that we at least have a capacity of 1
		auto new_capacity = vertex_count == 0 ? 1 : vertex_count;
		auto new_data = alloc.get().AllocateData(new_capacity * properties.VertexSize());
		memcpy(new_data, vertex_data, vertex_count * properties.VertexSize());
		vertex_data = new_data;
		owned_capacity = new_capacity;

		return *this;
	}

	uint32_t Count() const {
		return vertex_count;
	}

	uint32_t ByteSize() const {
		return vertex_count * properties.VertexSize();
	}

	uint32_t Capacity() const {
		if (IsOwning()) {
			return owned_capacity;
		} else {
			return vertex_count;
		}
	}

	const Properties &GetProperties() const {
		return properties;
	}

	const_data_ptr_t GetData() const {
		return vertex_data;
	}

	/// Return a new vertex array that is a slice of the current vertex array
	VertexArray Slice(uint32_t start, uint32_t count) const {
		D_ASSERT(start + count <= vertex_count);
		return VertexArray {alloc, vertex_data + start * properties.VertexSize(), count, properties.HasZ(),
		                    properties.HasM()};
	}

	// Zero-initialize the vertex array up to the current capacity
	VertexArray &Initialize(bool zero_initialize) {
		if (!IsOwning()) {
			MakeOwning();
		}
		if (zero_initialize) {
			auto remaining = owned_capacity - vertex_count;
			memset(vertex_data + vertex_count * properties.VertexSize(), 0, remaining * properties.VertexSize());
		}
		vertex_count = owned_capacity;
		return *this;
	}

	//-------------------------------------------------------------------------
	// Set
	//-------------------------------------------------------------------------

	// This requires the vertex type to be the exact same as the stored vertex type
	template <class V>
	void SetTemplatedUnsafe(uint32_t i, const V &v) {
		static_assert(V::IS_VERTEX, "V must be a vertex type");
		D_ASSERT(IsOwning());                    // Only owning arrays can be modified
		D_ASSERT(i < vertex_count);              // Index out of bounds
		D_ASSERT(properties.HasM() == V::HAS_M); // M mismatch
		D_ASSERT(properties.HasZ() == V::HAS_Z); // Z mismatch
		auto offset = i * sizeof(V);
		Store<V>(v, vertex_data + offset);
	}

	// This requires the vertex type to be the exact same as the stored vertex type
	template <class V>
	void SetTemplated(uint32_t i, const V &v) {
		if (!IsOwning()) {
			MakeOwning();
		}
		SetTemplatedUnsafe(i, v);
	}

	void SetUnsafe(uint32_t i, double x, double y) {
		D_ASSERT(IsOwning());         // Only owning arrays can be modified
		D_ASSERT(i < vertex_count);   // Index out of bounds
		D_ASSERT(!properties.HasZ()); // Z mismatch
		D_ASSERT(!properties.HasM()); // M mismatch
		auto offset = i * properties.VertexSize();
		Store<double>(x, vertex_data + offset);
		Store<double>(y, vertex_data + offset + sizeof(double));
	}

	void SetUnsafe(uint32_t i, double x, double y, double zm) {
		D_ASSERT(IsOwning());                             // Only owning arrays can be modified
		D_ASSERT(i < vertex_count);                       // Index out of bounds
		D_ASSERT(properties.HasZ() || properties.HasM()); // ZM mismatch
		auto offset = i * properties.VertexSize();
		Store<double>(x, vertex_data + offset);
		Store<double>(y, vertex_data + offset + sizeof(double));
		Store<double>(zm, vertex_data + offset + 2 * sizeof(double));
	}

	void SetUnsafe(uint32_t i, double x, double y, double z, double m) {
		D_ASSERT(IsOwning());                             // Only owning arrays can be modified
		D_ASSERT(i < vertex_count);                       // Index out of bounds
		D_ASSERT(properties.HasZ() && properties.HasM()); // ZM mismatch
		auto offset = i * properties.VertexSize();
		Store<double>(x, vertex_data + offset);
		Store<double>(y, vertex_data + offset + sizeof(double));
		Store<double>(z, vertex_data + offset + 2 * sizeof(double));
		Store<double>(m, vertex_data + offset + 3 * sizeof(double));
	}

	//-------------------------------------------------------------------------
	// Append
	//-------------------------------------------------------------------------

	void AppendUnsafe(const VertexXY &v) {
		D_ASSERT(IsOwning()); // Only owning arrays can be modified
		auto offset = vertex_count * properties.VertexSize();
		Store<VertexXY>(v, vertex_data + offset);
		vertex_count++;
	}

	void Append(const VertexXY &v) {
		if (!IsOwning()) {
			MakeOwning();
		}
		Reserve(vertex_count + 1);
		AppendUnsafe(v);
	}

    // Append one vertex array to the other. If the others vertex type doesnt match, performs a deep copy before appending.
    void Append(const VertexArray &other) {
        if (!IsOwning()) {
            MakeOwning();
        }

        auto vertex_size = properties.VertexSize();
        auto other_data = other.GetData();
        auto other_count = other.Count();
        auto other_vertex_size = other.properties.VertexSize();
        Reserve(vertex_count + other_count);


        auto this_has_z = properties.HasZ();
        auto this_has_m = properties.HasM();
        auto other_has_z = other.properties.HasZ();
        auto other_has_m = other.properties.HasM();

        // Fast path, properties are the same
        if(this_has_z == other_has_z && this_has_m == other_has_m) {
            memcpy(vertex_data + vertex_count * vertex_size, other_data, other_count * other_vertex_size);
            vertex_count += other_count;
        } else {
            // Ensure the vertex types match by copying and resizing the other array
            // TODO: Optimize this, we can avoid the copy if the vertex sizes fit.
            VertexArray other_copy = other;
            other_copy.UpdateVertexType(this_has_z, this_has_m);

            other_data = other_copy.GetData();
            other_count = other_copy.Count();
            other_vertex_size = other_copy.GetProperties().VertexSize();
            memcpy(vertex_data + vertex_count * vertex_size, other_data, other_count * other_vertex_size);
            vertex_count += other_count;
        }
    }

	//-------------------------------------------------------------------------
	// Reserve
	//-------------------------------------------------------------------------
	VertexArray &Reserve(uint32_t count) {
		if (count > owned_capacity) {
			if (owned_capacity == 0) {
				// TODO: Optimize this, we can allocate the exact amount of memory we need
				MakeOwning();
			}
			auto vertex_size = properties.VertexSize();
			vertex_data = alloc.get().ReallocateData(vertex_data, owned_capacity * vertex_size, count * vertex_size);
			owned_capacity = count;
		}
		return *this;
	}

	//-------------------------------------------------------------------------
	// Get
	//-------------------------------------------------------------------------

	// This requires the vertex type to be the exact same as the stored vertex type
	template <class V>
	V GetTemplated(uint32_t i) const {
		static_assert(V::IS_VERTEX, "V must be a vertex type");
		D_ASSERT(i < vertex_count);              // Index out of bounds
		D_ASSERT(properties.HasM() == V::HAS_M); // M mismatch
		D_ASSERT(properties.HasZ() == V::HAS_Z); // Z mismatch
		auto offset = i * sizeof(V);
		return Load<V>(vertex_data + offset);
	}

	// This is safe to call on XYZ, XYM and XYZM arrays too
	VertexXY Get(uint32_t i) const {
		D_ASSERT(i < vertex_count); // Index out of bounds
		auto offset = i * properties.VertexSize();
		return Load<VertexXY>(vertex_data + offset);
	}

	//-------------------------------------------------------------------------
	// Length
	//-------------------------------------------------------------------------
	// TODO: Get rid of this
	double Length() const {
		double length = 0;
		if (Count() < 2) {
			return 0.0;
		}
		for (uint32_t i = 0; i < Count() - 1; i++) {
			auto p1 = Get(i);
			auto p2 = Get(i + 1);
			length += std::sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
		}
		return length;
	}

	bool IsClosed() const {
		if (Count() == 0) {
			return false;
		}
		if (Count() == 1) {
			return true;
		}
		// TODO: Check approximate equality?
		auto start = Get(0);
		auto end = Get(Count() - 1);
		return start.x == end.x && start.y == end.y;
	}

	bool IsEmpty() const {
		return Count() == 0;
	}

	//-------------------------------------------------------------------------
	// Change dimensions
	//-------------------------------------------------------------------------
	// TODO: Test this properly
	void UpdateVertexType(bool has_z, bool has_m, double default_z = 0, double default_m = 0) {
		if (properties.HasZ() == has_z && properties.HasM() == has_m) {
			return;
		}
		if (!IsOwning()) {
			MakeOwning();
		}
		auto used_to_have_z = properties.HasZ();
		auto used_to_have_m = properties.HasM();
		auto old_vertex_size = properties.VertexSize();
		properties.SetZ(has_z);
		properties.SetM(has_m);

		auto new_vertex_size = properties.VertexSize();
		// Case 1: The new vertex size is larger than the old vertex size
		if (new_vertex_size > old_vertex_size) {
			vertex_data =
			    alloc.get().ReallocateData(vertex_data, vertex_count * old_vertex_size, vertex_count * new_vertex_size);

			// There are 5 cases here:
			if (used_to_have_m && has_m && !used_to_have_z && has_z) {
				// 1. We go from XYM to XYZM
				// This is special, because we need to slide the M value to the end of each vertex
				for (int64_t i = vertex_count - 1; i >= 0; i--) {
					auto old_offset = i * old_vertex_size;
					auto new_offset = i * new_vertex_size;
					memcpy(vertex_data + new_offset, vertex_data + old_offset, sizeof(double) * 2);
					auto z_offset = old_offset + sizeof(double) * 2;
					auto m_offset = old_offset + sizeof(double) * 3;
					memcpy(vertex_data + new_offset + m_offset, vertex_data + z_offset, sizeof(double));
					memcpy(vertex_data + new_offset + z_offset, &default_z, sizeof(double));
				}
			} else if (!used_to_have_z && has_z && !used_to_have_m && has_m) {
				// 2. We go from XY to XYZM
				// This is special, because we need to add both the default Z and M values to the end of each vertex
				for (int64_t i = vertex_count - 1; i >= 0; i--) {
					auto old_offset = i * old_vertex_size;
					auto new_offset = i * new_vertex_size;
					memcpy(vertex_data + new_offset, vertex_data + old_offset, sizeof(double) * 2);
					memcpy(vertex_data + new_offset + sizeof(double) * 2, &default_z, sizeof(double));
					memcpy(vertex_data + new_offset + sizeof(double) * 3, &default_m, sizeof(double));
				}
			} else {
				// Otherwise:
				// 3. We go from XY to XYZ
				// 4. We go from XY to XYM
				// 5. We go from XYZ to XYZM
				// These are all really the same, we just add the default to the end
				auto default_value = has_m ? default_m : default_z;
				for (int64_t i = vertex_count - 1; i >= 0; i--) {
					auto old_offset = i * old_vertex_size;
					auto new_offset = i * new_vertex_size;
					memmove(vertex_data + new_offset, vertex_data + old_offset, old_vertex_size);
					memcpy(vertex_data + new_offset + old_vertex_size, &default_value, sizeof(double));
				}
			}
		}
		// Case 2: The new vertex size is equal to the old vertex size
		else if (new_vertex_size == old_vertex_size) {
			// This only happens when we go from XYZ -> XYM or XYM -> XYZ
			// In this case we just need to set the default on the third dimension
			auto default_value = has_m ? default_m : default_z;
			for (uint32_t i = 0; i < vertex_count; i++) {
				auto offset = i * new_vertex_size + sizeof(double) * 2;
				memcpy(vertex_data + offset, &default_value, sizeof(double));
			}
		}
		// Case 3: The new vertex size is smaller than the old vertex size.
		// In this case we need to allocate new memory and copy the data over to not lose any data
		else {
			auto new_data = alloc.get().AllocateData(vertex_count * new_vertex_size);

			// Special case: If we go from XYZM to XYM, we need to slide the M value to the end of each vertex
			if (used_to_have_z && used_to_have_m && !has_z && has_m) {
				for (uint32_t i = 0; i < vertex_count; i++) {
					auto old_offset = i * old_vertex_size;
					auto new_offset = i * new_vertex_size;
					memcpy(new_data + new_offset, vertex_data + old_offset, sizeof(double) * 2);
					auto m_offset = old_offset + sizeof(double) * 3;
					memcpy(new_data + new_offset + sizeof(double) * 2, vertex_data + m_offset, sizeof(double));
				}
			} else {
				// Otherwise, we just copy the data over
				for (uint32_t i = 0; i < vertex_count; i++) {
					auto old_offset = i * old_vertex_size;
					auto new_offset = i * new_vertex_size;
					memcpy(new_data + new_offset, vertex_data + old_offset, new_vertex_size);
				}
			}
			alloc.get().FreeData(vertex_data, vertex_count * old_vertex_size);
			vertex_data = new_data;
		}
	}
    //-------------------------------------------------------------------------
    // ToString()
    //-------------------------------------------------------------------------

    string ToString() const {
        auto has_z = properties.HasZ();
        auto has_m = properties.HasM();
        if(has_z && has_m) {
            string result = StringUtil::Format("VertexArray XYZM (%d/%d) [", vertex_count, owned_capacity);
            for (uint32_t i = 0; i < vertex_count; i++) {
                auto vertex = GetTemplated<VertexXYZM>(i);
                result += StringUtil::Format("(%f, %f, %f, %f)", vertex.x, vertex.y, vertex.z, vertex.m);
                if(i < vertex_count - 1) {
                    result += ", ";
                }
            }
            result += "]";
            return result;
        } else if(has_z) {
            string result = StringUtil::Format("VertexArray XYZ (%d/%d) [", vertex_count, owned_capacity);
            for (uint32_t i = 0; i < vertex_count; i++) {
                auto vertex = GetTemplated<VertexXYZ>(i);
                result += StringUtil::Format("(%f, %f, %f)", vertex.x, vertex.y, vertex.z);
                if(i < vertex_count - 1) {
                    result += ", ";
                }
            }
            result += "]";
            return result;
        } else if(has_m) {
            string result = StringUtil::Format("VertexArray XYM (%d/%d) [", vertex_count, owned_capacity);
            for (uint32_t i = 0; i < vertex_count; i++) {
                auto vertex = GetTemplated<VertexXYM>(i);
                result += StringUtil::Format("(%f, %f, %f)", vertex.x, vertex.y, vertex.m);
                if(i < vertex_count - 1) {
                    result += ", ";
                }
            }
            result += "]";
            return result;
        } else {
            string result = StringUtil::Format("VertexArray XY (%d/%d) [", vertex_count, owned_capacity);
            for (uint32_t i = 0; i < vertex_count; i++) {
                auto vertex = GetTemplated<VertexXY>(i);
                result += StringUtil::Format("(%f, %f)", vertex.x, vertex.y);
                if(i < vertex_count - 1) {
                    result += ", ";
                }
            }
            result += "]";
            return result;
        }
    }
};

} // namespace core

} // namespace spatial