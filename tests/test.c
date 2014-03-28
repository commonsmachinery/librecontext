#include <recontext.h>

void main()
{
    recontext* rc;

    recontext_init();

    rc = recontext_new();

    recontext_cleanup();
}
