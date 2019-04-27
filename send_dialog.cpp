//
// Created by Lev on 12.02.2019.
//

#include "send_dialog.h"
#include "resource.h"
RUN(tftp::send_dialog);

tftp::send_dialog::send_dialog() {
    setup.dialogId = IDD_SEND;

    on_message(WM_INITDIALOG, [this](params)->LRESULT {
        list1.assign(this, IDC_SEND);
        list1.imageList16.load_from_resource(IDI_ICON1);
        list1.imageList32.load_from_resource(IDI_ICON1);
        list1.columns.add(L"Название", 125);
        list1.columns.add(L"IP Адрес", 90);
        return 0;
    });

    on_command(IDOK, [this](wm::command p)->INT_PTR {
        list1.items.add(L"localhost");
        list1.items.get_last().set_text(L"127.0.0.1",1);
        return 0;
    });
}
