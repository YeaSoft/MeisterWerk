// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

namespace meisterwerk {
    namespace core {

        enum T_PRIO {
            PRIORITY_SYSTEMCRITICAL = 0,
            PRIORITY_TIMECRITICAL   = 1,
            PRIORITY_HIGH           = 2,
            PRIORITY_NORMAL         = 3,
            PRIORITY_LOW            = 4,
            PRIORITY_LOWEST         = 5
        };
    } // namespace core
} // namespace meisterwerk
