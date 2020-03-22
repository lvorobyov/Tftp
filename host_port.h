//
// Created by lev on 04.03.2020.
//

#ifndef STRATUM_PROXY_HOST_PORT_H
#define STRATUM_PROXY_HOST_PORT_H

#include <string>

namespace btc {
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

#endif //STRATUM_PROXY_HOST_PORT_H
