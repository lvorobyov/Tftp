//
// Created by Lev on 19.04.2019.
//

#ifndef _WRITER_H_
#define _WRITER_H_

#include <win32/thread.h>
#include <win32/waitable.h>
#include "connection.h"

namespace tftp {

    using namespace csoi::win32;

    template <template <typename> class Container>
    class writer : public thread<writer<Container>> {
    private:
        Container<connection> &connections;
        event e_shutdown;
        event e_update;
        fiber_primary &other;

    public:
        writer(Container<connection> &connections, fiber_primary &other);

        void on_change();

        void stop();

    protected:
        DWORD thread_main() noexcept override;
    };

    template<template <typename> class Container>
    writer<Container>::writer(Container<connection> &connections, fiber_primary &other)
        : connections(connections), other(other) { }

    template<template <typename> class Container>
    inline DWORD tftp::writer<Container>::thread_main() noexcept {
        vector<HANDLE> handles(2);
        handles[0] = e_shutdown.raw_handle();
        handles[1] = e_update.raw_handle();
        try {
            fiber_primary primary;
            for (auto &c: connections)
                c.set_auxiliary(primary);
            do {
                // Wait any received data
                auto h = wait(false, handles.begin(), handles.end());
                if (h == 0)
                    break;
                if (h == 1) {
                    // Update handles
                    handles.resize(2);
                    for (auto const &conn: connections)
                        handles.push_back(conn.get_received().raw_handle());
                } else {
                    auto fib = connections.begin();
                    advance(fib, h - 2);
                    // Detect is concurrent thread interposed
                    if (fib->is_writing()) {
                        primary.switch_to(other);
                    } else {
                        // Write data into file
                        fib->set_written();
                        fib->yield_from(primary);
                    }
                }
            } while (true);
        } catch (exception const& ex) {
            return 1;
        }
        return 0;
    }

    template<template <typename> class Container>
    void writer<Container>::on_change() {
        e_update.set();
    }

    template<template <typename> class Container>
    void writer<Container>::stop() {
        e_shutdown.set();
        thread<writer>::wait();
    }

}

#endif //_WRITER_H_
