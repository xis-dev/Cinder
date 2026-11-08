#pragma once

#include "CinUtility.h"

#include <cstdint>

struct Component {
public:
     virtual ~Component() = default;
};

typedef uint32_t ComponentTypeID;

namespace ComponentUtil
{
     inline ComponentTypeID nextComponentID{};

     template<CinUtility::DerivedConcept<Component> T>
     ComponentTypeID getComponentID()
     {
          //static ComponentTypeID id = []()->ComponentTypeID {
            //   ComponentTypeID i = 1 << nextComponentShift;
              // ++nextComponentShift;
               //return i;
          //};

          static ComponentTypeID id = nextComponentID++;
          return id;
     }
}
