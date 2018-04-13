#include "new.h"
#include "stdint.h"
#include "stdlib.h"
#include "time.h"
#include "Protocol/Common.h"
#include "Test/TransportProtocolTest.h"

int main(int argc, char* argv[])
{
    TransportProtocolTest test;
    if (!test.CreateBody(1024))
        return -1;

    bool r = test.Run();
	return 0;
}

