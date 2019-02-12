//
// Created by Lev on 12.02.2019.
//

#include "send_dialog.h"
#include "resource.h"

tftp::send_dialog::send_dialog() {
    setup.dialogId = IDD_SEND;

    on_message(WM_INITDIALOG, [this](params)->LRESULT {
        list1.assign(this, IDC_LIST1);
        return 0;
    });
}
