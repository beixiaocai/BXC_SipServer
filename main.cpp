//
// Created bxc on 2022/11/25.
//

#include "SipServer.h"
#include "Utils/Log.h"
int main(int argc, char *argv[]) {
    LOGI("");

    ServerInfo info(
            "BXC_SipServer",
            "1234567890123456",
            "192.168.1.3",
            15060,
            10000,
            "34020000002000000001",
            "3402000000",
            "123456789",
            1800,
            3600);

    SipServer sipServer(&info);
    sipServer.loop();

    return 0;
}