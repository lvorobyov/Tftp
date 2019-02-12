//
// Created by Lev on 12.02.2019.
//

#ifndef TFTP_SEND_DIALOG_H
#define TFTP_SEND_DIALOG_H

#include <winlamb/dialog_main.h>
#include <winlamb/listview.h>

namespace tftp {

    using namespace wl;

    class send_dialog : public dialog_main {
    private:
        listview list1;
    public:
        send_dialog();
    };

}


#endif //TFTP_SEND_DIALOG_H
