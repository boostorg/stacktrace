
#include <dlfcn.h>
#include <execinfo.h>

int main() {
    void* buffer[10];
    ::backtrace(buffer, 10);

}
