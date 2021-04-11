// Copyright (c) 2020, Лев Воробьёв
// This file is licensed under GPL v3

#ifndef _HOST_PORT_H
#define _HOST_PORT_H

#include <string>

namespace utils {
    using namespace std;

    class host_port {
    private:
        string _host;
        u_short _port;

    public:
        /**
         * Разбор строки host:port
         * @param str строка для разбора
         * @throws invalid_argument когда номер порта содержит что-либо кроме цифр либо превышает допустимое значение
         * @note если в строке отсутствует двоеточие, то считается, что вся строка содержит только номер порта
         */
        explicit host_port(const char *str);

        [[nodiscard]] const string &get_host() const;

        [[nodiscard]] u_short get_port() const;
    };

}

#endif //_HOST_PORT_H
