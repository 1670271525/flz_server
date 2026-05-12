#include "http/ws_connection.h"
#include "include/iomanager.h"
#include "include/util.h"


void run() {
    auto rt = flz::http::WSConnection::Create("http://127.0.0.1:8020/flz", 1000);
    if(!rt.second) {
        std::cout << rt.first->toString() << std::endl;
        return;
    }

    auto conn = rt.second;
    while(true) {
        //for(int i = 0; i < 1100; ++i) {
        for(int i = 0; i < 1; ++i) {
            conn->sendMessage(flz::random_string(60), flz::http::WSFrameHead::TEXT_FRAME, false);
        }
        conn->sendMessage(flz::random_string(65), flz::http::WSFrameHead::TEXT_FRAME, true);
        auto msg = conn->recvMessage();
        if(!msg) {
            break;
        }
        std::cout << "opcode=" << msg->getOpcode()
                  << " data=" << msg->getData() << std::endl;

        sleep(10);
    }
}

int main(int argc, char** argv) {
    srand(time(0));
    flz::IOManager iom(1);
    iom.schedule(run);
    return 0;
}


