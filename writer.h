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

    public:
        explicit writer(Container<connection> &connections);

        void on_change();

        void stop();

    protected:
        DWORD thread_main() noexcept override;
    };

    template<template <typename> class Container>
    writer<Container>::writer(Container<connection> &connections) : connections(connections) { }

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
                auto h = wait(false, handles.begin(), handles.end());
                if (h == 0)
                    break;
                if (h == 1) {
                    handles.resize(2);
                    for (auto const &conn: connections)
                        handles.push_back(conn.get_received().raw_handle());
                } else {
                    auto fib = connections.begin();
                    advance(fib, h - 2);
                    fib->get_guard().wait();
                    primary.switch_to(*fib);
                    fib->get_guard().release();
                }
            } while (true);
            // Wait any received data
            // Write data into file
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
