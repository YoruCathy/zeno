#include <zeno/zeno.h>
#include <spdlog/spdlog.h>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#ifdef __linux__
#include <string.h>
#endif

namespace zeno {

// defined in zeno/utils/StackTraceback.cpp:
void print_traceback(int skip);

static const char *signal_to_string(int signo) {
#ifdef __linux__
    return strsignal(signo);
#else
    const char *signame = "SIG-unknown";
    if (signo == SIGSEGV) signame = "SIGSEGV";
    if (signo == SIGFPE) signame = "SIGFPE";
    if (signo == SIGILL) signame = "SIGILL";
    return signo;
#endif
}

class SignalException : public std::exception {
private:
    int signo;

public:
    SignalException(int signo) noexcept : signo(signo) {
        spdlog::error("recieved signal {}: {}", signo, signal_to_string(signo));
        print_traceback(1);
    }

    ~SignalException() noexcept = default;

    char const *what() noexcept { return signal_to_string(signo); }
};

#ifdef ZENO_FAULTHANDLER
static void signal_handler(int signo) {
    throw SignalException(signo);
}

static int register_my_handlers() {
    if (getenv("ZEN_NOSIGHOOK")) {
        return 0;
    }
    signal(SIGSEGV, signal_handler);
    signal(SIGFPE, signal_handler);
    signal(SIGILL, signal_handler);
#ifdef __linux__
    signal(SIGBUS, signal_handler);
#endif
    return 1;
}

static int doRegisterMyHandlers = register_my_handlers();
#endif

}
