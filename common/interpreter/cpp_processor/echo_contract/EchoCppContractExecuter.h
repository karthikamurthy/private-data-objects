#pragma once
#define STUB_INTERPRETOR_NO_ERROR (0)
#define STUB_INTERPRETOR_ERR (1)
#define STUB_INTERPRETOR_ERR_CODE (2)
#define STUB_INTERPRETOR_ERR_MESSAGE (3)
#define STUB_INTERPRETOR_ERR_STATE (4)
#define STUB_INTERPRETOR_ERR_PARAM (5)
#define STUB_INTERPRETOR_ERR_TERMINATED (6)
#define STUB_INTERPRETOR_ERR_RESULT (7)
#define STUB_INTERPRETOR_ERR_STRING_NULL (8)
#define STUB_INTERPRETOR_ERR_STRING_TO_INT (9)

#ifndef NULL
#define NULL (0)
#endif

#include "CppProcessorHandler.h"

struct EchoCode
{
    unsigned int code_value;

    EchoCode()
    {
        code_value= 0;
    };

    int Init(const char* str);
};

struct EchoMessage
{
    unsigned int value;

    EchoMessage()
    {
        value = 0;
    };
    int Init(const char* str);
};

struct EchoState
{
    unsigned int terminated;
    unsigned int value;

    EchoState()
    {
        terminated = 0;
        value = 0;
    }

    int Init(const char* str);
    int Serialize(char* buf, int bufSize);
};

class EchoCppContractException : public CppContractWrapperException
{
public:
    EchoCppContractException(const char* msg) :
    CppContractWrapperException(msg)
    {}
    virtual char const* what() const noexcept { return msg_.c_str(); }
};


class EchoCppContractExecuter : public CppContractWrapper
{
public:
    EchoCppContractExecuter() { result = STUB_INTERPRETOR_NO_ERROR; };

    bool SetCode(const char* codeStr)
    {
        if (result == STUB_INTERPRETOR_NO_ERROR)
            result = code.Init(codeStr);
        return (result == STUB_INTERPRETOR_NO_ERROR);
    };

    bool SetMessage(const char* messageStr, const char* originatorId)
    {
        // TODO: originatorId is not used
        if (result == STUB_INTERPRETOR_NO_ERROR)
            result = message.Init(messageStr);
        return (result == STUB_INTERPRETOR_NO_ERROR);
    };

    bool SetInState(const char* stateStr)
    {
        if (result == STUB_INTERPRETOR_NO_ERROR)
            result = state.Init(stateStr);
        ;
        return (result == STUB_INTERPRETOR_NO_ERROR);
    };

    bool ExecuteMessage(const char* contractId, const char* creatorId);

    bool GetResult(char* buf, int bufSize);

    bool GetOutState(char* buf, int bufSize)
    {
        return (state.Serialize(buf, bufSize) == STUB_INTERPRETOR_NO_ERROR);
    }

    void HandleFailure(const char* msg);

    // TODO: GetDependencies()

private:
    EchoCode code;
    EchoState state;
    EchoMessage message;
    int result;
};
