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

    on_command({IDOK,IDCANCEL}, [this](wm::command p)->BOOL {
        if (p.control_id() != IDOK)
            DestroyWindow(hwnd());
        return TRUE;
    });
}

void tftp::send_dialog::add_peer(const wchar_t *host, const wchar_t *addr) {
    this->list1.items.add(host);
    this->list1.items.get_last().set_text(addr, 1);
}
