# CTX

CTX is a basic modified [STB-Style](https://github.com/nothings/stb/blob/master/docs/stb_howto.txt) library for basic memory management using contexts/arenas.

## Building Examples

Linux/MacOS Steps:
```bash
cc -Wall -Wextra -Isrc example/main.c -o application

./application
```

Windows Steps: *
```powershell
cl /Feapplication.exe /I"src" example/main.c

.\application.exe
```

> _**Note:** All Windows testing was performed using [PortableBuildTools](https://github.com/Data-Oriented-House/PortableBuildTools) and not tested using Visual Studio_

## Example Code
```c
// main.c
#define CTX_IMPL
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

```
