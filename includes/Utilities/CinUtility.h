#pragma once

#include <concepts>

namespace CinUtility
{
    template<typename T, typename Base>
    concept DerivedConcept = std::derived_from<T, Base>;
}