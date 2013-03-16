#include <stdarg.h>
 
static void f(va_list *ap) {
}
 
static void g(va_list ap) {
        f(&ap);
}
 
int main(void) {
        va_list ap;
        g(ap);
        return 0;
}

