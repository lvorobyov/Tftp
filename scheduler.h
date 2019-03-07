//
// Created by Lev on 26.02.2019.
//

#ifndef TFTP_SCHEDULER_H
#define TFTP_SCHEDULER_H

#include <win32/thread.h>
#include <win32/event.h>
#include <win32/waitable.h>
#include <forward_list>
#include <vector>

namespace tftp {

    using namespace csoi::win32;
    using namespace std;

    template <typename T>
    class scheduler : public thread<scheduler<T>> {
    private:
        event e_incoming;
        event e_shutdown;
        vector<T*> runners;

    public:
        void execute(T* runner);

        void shutdown();

    protected:
        DWORD thread_main() noexcept override;
    };

    template<typename T>
    void scheduler<T>::execute(T* runner) {
        runners.push_back(runner);
        e_incoming.set();
    }

    template<typename T>
    void scheduler<T>::shutdown() {
        e_shutdown.set();
    }

    template<typename T>
    DWORD scheduler<T>::thread_main() noexcept {
        DWORD status = 0;
        forward_list<object*> events({&e_shutdown,&e_incoming});
        try {
            while (auto o = wait_any(events.begin(), events.end())) {
                if (o == &e_shutdown) {
                    break;
                } else if (o == &e_incoming) {
                    auto thread = runners.back();
                    runners.pop_back();
                    thread->start();
                    events.push_front(thread);
                } else {
                    // handle thread finish
                    events.remove(o);
                    delete o;
                }
            }
        } catch (exception const& ex) {
            cerr << ex.what() << endl;
            status = 1;
        }
        events.reverse();
        auto it = events.begin();
        advance(it, 2);
        wait_all(it, events.end());
        for_each(it, events.end(), [](auto i) {
            delete i;
        });
        return status;
    }

}


#endif //TFTP_SCHEDULER_H
