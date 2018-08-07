#pragma once

#include <map>
#include <string>
using namespace std;
#include "ContractInterpreter.h"
#include "CppProcessor.h"
#include "IntKeyCppContractExecuter.h"
namespace pc = pdo::contracts;

pdo::contracts::ContractInterpreter* intkey_factory();
pdo::contracts::ContractInterpreter* echo_factory();

class CppContractWrapperException : public std::exception
{
public:
    CppContractWrapperException(const char* msg) : msg_(msg) {}
    virtual char const* what() const noexcept { return msg_.c_str(); }

private:
    std::string msg_;
};

class CppContractWrapper : public pc::ContractInterpreter
{
public:
    CppContractWrapper(void);
    ~CppContractWrapper(void);

    virtual void create_initial_contract_state(const std::string& inContractID,
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

private:
    void HandleFailure();
    bool HandleFailure_Code();
    bool HandleFailure_Message();
    bool HandleFailure_State();
    IntKeyCppContractExecuter executer;

};
