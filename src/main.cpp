#include "new.h"
#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include "Protocol/Common.h"
#include "Test/TransportProtocolTest.h"

int main(int argc, char* argv[])
{
    TransportProtocolTest test;

    bool r = test.Test();
    if (r)
        printf("success test\n");
    else
        printf("failed test\n");

	return 0;
}

