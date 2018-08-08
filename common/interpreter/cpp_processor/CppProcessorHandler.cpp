#include <stdio.h>
#include <stdlib.h>

#include "CppProcessorHandler.h"
#include "intkey_contract/IntKeyCppContractExecuter.h"
#include "echo_contract/EchoCppContractExecuter.h"

extern "C" {
void printf(const char* fmt, ...);
}

CppContractWrapper* intkey_factory()
{
    return new IntKeyCppContractExecuter();
}

CppContractWrapper* echo_factory()
{
    return new EchoCppContractExecuter();
}

