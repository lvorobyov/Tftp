//
// Created by Lev on 26.02.2019.
//

#include "waitable_list.h"
#include <vector>
using namespace tftp;

tftp::waitable_list::waitable_list(const initializer_list<reference_wrapper<object>> &init) {
    for (auto &i: init) {
        m_handles.push_back(i.get().raw_handle());
    }
}

DWORD tftp::waitable_list::wait(bool all, DWORD timeout) {
    if (m_handles.empty())
        return WAIT_FAILED;
    vector<HANDLE> handles(m_handles.begin(), m_handles.end());
    DWORD result = ::WaitForMultipleObjects(m_handles.size(), handles.data(), all, timeout);
    switch (result) {
        case WAIT_FAILED:
            raise_exception("WaitForMultipleObjects");
        case WAIT_TIMEOUT:
            return result;
        default:
            return (result >= WAIT_ABANDONED)
                   ? result - WAIT_ABANDONED
                   : result - WAIT_OBJECT_0;
    }
}

void tftp::waitable_list::add(const object &obj) {
    m_handles.push_back(obj.raw_handle());
}

void tftp::waitable_list::remove(const object &obj) {
    m_handles.remove(obj.raw_handle());
}
