#pragma once

#include <cyphal/definitions.h>
#include <libcanard/canard.h>
#include <tuple>
#include <type_traits>


class AbstractAllocator {
protected:
    UtilityConfig& utilities;
public:
    AbstractAllocator(size_t size, UtilityConfig& utilities): utilities(utilities) {};
    virtual void* allocate(CanardInstance* ins, size_t amount) = 0;
    virtual void free(CanardInstance* ins, void* pointer) = 0;
    virtual ~AbstractAllocator() {}
};
