//
// Created by Lev on 19.04.2019.
//

#include "writer.h"

DWORD tftp::writer::thread_main() noexcept {
    try {
        fiber_primary primary;
        // Wait any received data
        // Write data into file
    } catch (exception const& ex) {
        return 1;
    }
    return 0;
}
