#define CTX_IMPLEMENTATION
#include "ctx.h"

int main (void) {
    // Static Context
    Context ctx = new_context (1 KB);
    int* life   = context_alloc (&ctx, sizeof (int));
    *life       = 42;

    CTX_LOG ("[STATIC]: life    = %d\n", *life);

    // Temp Context
    int* unlucky = context_talloc (sizeof (int));
    *unlucky     = 13;

    CTX_LOG ("[TEMP]:   unlucky = %d\n", *unlucky);

    context_free (&ctx);
    context_tfree ();

    return 0;
}
