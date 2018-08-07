#pragma once

#include <map>
#include <string>
#include <memory>
//using namespace std;
//#include "ContractInterpreter.h"
//#include "CppProcessor.h"
//#include "CppContractExecuter.h"
//namespace pc = pdo::contracts;

#define MIN_RESULT_BUFFER_SIZE (100)
#define MIN_STATE_BUFFER_SIZE (13)

class CppContractWrapper; // Forward decleration

CppContractWrapper* intkey_factory();
CppContractWrapper* echo_factory();

class CppContractWrapperException : public std::exception
{
public:
    CppContractWrapperException(const char* msg) : msg_(msg) {}
    virtual char const* what() const noexcept { return msg_.c_str(); }

protected:
    std::string msg_;
};

class CppContractWrapper
{
public:
    CppContractWrapper(){}

    virtual bool SetCode(const char* codeStr) = 0;
    virtual bool SetMessage(const char* messageStr, const char* originatorId) = 0;
    virtual bool SetInState(const char* stateStr) = 0;
    virtual bool ExecuteMessage(const char* contractId, const char* creatorId) = 0;
    virtual bool GetResult(char* buf, int bufSize) = 0;
    virtual bool GetOutState(char* buf, int bufSize) = 0;
    virtual void HandleFailure(const char* msg);

    virtual ~CppContractWrapper() {
    }

    /*virtual void create_initial_contract_state(const std::string& inContractID,
        const std::string& inCreatorID,
        const pc::ContractCode& inContract,
        const pc::ContractMessage& inMessage,
        pc::ContractState& outContractState);

    virtual void send_message_to_contract(const std::string& inContractID,
        const std::string& inCreatorID,
        const pc::ContractCode& inContract,
        const pc::ContractMessage& inMessage,
        const pc::ContractState& inContractState,
        pc::ContractState& outContractState,
        std::map<std::string, std::string>& outDependencies,
        std::string& outMessageResult);
        */
};
