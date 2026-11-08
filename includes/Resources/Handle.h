#pragma once

#include <cstdint>

#include <optional>

template <typename T>
struct Handle {
	std::uint32_t id{ 0 };

	static Handle InvalidHandle ;

	constexpr bool operator==(const Handle& handle) const{
		return id == handle.id;
	}

};

template<typename T>
Handle<T> Handle<T>::InvalidHandle = {UINT32_MAX};

// Handle hash function
namespace std {
	template<typename T>
	struct hash<Handle<T>> {
		size_t operator()(const Handle<T>& handle) const {
			return hash<uint32_t>{}(handle.id);
		}
	};
}

