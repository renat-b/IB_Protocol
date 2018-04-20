#include "new.h"
#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include "Protocol/Common.h"
#include "Protocol/CommonLibEmPulse/IndigoTimer.h"
#include "Test/TransportProtocolTest.h"

int main(int argc, char* argv[])
{
    IndigoTimer timer;
    TransportProtocolTest test;
    
    timer.Start(0);

    bool r = test.Test();
    if (r)
        printf("success test, %.2f sec\n", (double(timer.Stop()) / 1000));
    else
        printf("failed test\n");

	return 0;
}

