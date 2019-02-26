//
// Created by Lev on 26.02.2019.
//

#ifndef TFTP_WAITABLE_LIST_H
#define TFTP_WAITABLE_LIST_H

#include <win32/object.h>
#include <initializer_list>
#include <functional>
#include <list>

namespace tftp {

    using namespace std;
    using namespace csoi::win32;

    class waitable_list {
    private:
        list<HANDLE> m_handles;

    public:
        waitable_list(initializer_list<reference_wrapper<object>> const& init);

        DWORD wait(bool all, DWORD timeout = INFINITE);

        void add(const object &obj);

        void remove(const object &obj);
    };

}


#endif //TFTP_WAITABLE_LIST_H
