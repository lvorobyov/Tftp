//
// Created by Lev on 19.04.2019.
//

#ifndef _WRITER_H_
#define _WRITER_H_

#include <win32/thread.h>
#include "connection.h"

namespace tftp {

    using namespace csoi::win32;

    template <template <typename> class Container>
    class writer : public thread<writer<Container>> {
    private:
        Container<connection> &connections;

    public:
        explicit writer(Container<connection> &connections);

    protected:
        DWORD thread_main() noexcept override;
    };

    template<template <typename> class Container>
    writer<Container>::writer(Container<connection> &connections) : connections(connections) { }

    template<template <typename> class Container>
    inline DWORD tftp::writer<Container>::thread_main() noexcept {
        try {
            fiber_primary primary;
            // Wait any received data
            // Write data into file
        } catch (exception const& ex) {
            return 1;
        }
        return 0;
    }

}

#endif //_WRITER_H_
