#pragma once

#include <concepts>

namespace CinUtility
{
    template<typename Base, typename T>
    concept DerivedConcept = std::derived_from<T, Base>;
}