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
        list1.columns.add(L"IP Адрес", 100);
        return 0;
    });

    on_command(IDOK, [this](wm::command p)->INT_PTR {
        add_peer(L"localhost", L"127.0.0.1");
        add_peer(L"broadcast", L"255.255.255.255");
        return 0;
    });
}

void tftp::send_dialog::add_peer(const wchar_t *host, const wchar_t *addr) {
    this->list1.items.add(host);
    this->list1.items.get_last().set_text(addr, 1);
}
