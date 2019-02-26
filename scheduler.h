//
// Created by Lev on 26.02.2019.
//

#ifndef TFTP_SCHEDULER_H
#define TFTP_SCHEDULER_H

#include <win32/thread.h>
#include <win32/event.h>
#include <vector>
#include <memory>

#include "waitable_list.h"

namespace tftp {

    using namespace csoi::win32;
    using namespace std;

    template <typename T>
    class scheduler : public thread<scheduler<T>> {
    private:
        event e_incoming;
        event e_shutdown;
        vector<shared_ptr<T>> runners;

    public:
        void execute(shared_ptr<T> & runner);

        void shutdown();

    protected:
        DWORD thread_main() noexcept override;
    };

    template<typename T>
    void scheduler<T>::execute(shared_ptr<T> &runner) {
        runners.push_back(runner);
        e_incoming.set();
    }

    template<typename T>
    void scheduler<T>::shutdown() {
        e_shutdown.set();
    }

    template<typename T>
    DWORD scheduler<T>::thread_main() noexcept {
        waitable_list events({e_shutdown,e_incoming});
        DWORD index = 0;
        try {
            while ((index = events.wait(false)) != 0) {
                if (index == 1) {
                    auto &thread = *runners.back().get();
                    thread.start();
                    events.add(thread);
                } else {
                    // handle thread finish
                    events.remove(*runners[index - 2]);
                    runners.erase(runners.begin() + index - 2);
                }
            }
            events.remove(e_shutdown);
            events.remove(e_incoming);
            events.wait(true);
        } catch (exception const& ex) {
            cerr << ex.what() << endl;
        }
        return 0;
    }

}


#endif //TFTP_SCHEDULER_H
