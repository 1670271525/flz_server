#pragma once
#ifndef __FLZ_LIBRARY_H__
#define __FLZ_LIBRARY_H__

#include <memory>
#include "module.h"

namespace flz {


class Library {
public:
    static Module::ptr GetModule(const std::string& path);
};



}

#endif
