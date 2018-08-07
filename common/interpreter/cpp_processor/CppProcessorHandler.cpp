#include <stdio.h>
#include <stdlib.h>

#include "CppProcessorHandler.h"
#include "intkey_contract/IntKeyCppContractExecuter.h"

extern "C" {
void printf(const char* fmt, ...);
}

CppContractWrapper* intkey_factory()
{
   return new IntKeyCppContractExecuter();
}

CppContractWrapper* echo_factory(){

}

/*void CppContractWrapper::create_initial_contract_state(const std::string& inContractID,
    const std::string& inCreatorID,
    const pc::ContractCode& inContract,
    const pc::ContractMessage& inMessage,
    pc::ContractState& outContractState)
{

    if (!executer->SetCode(inContract.Code.c_str()))
        throw CppContractWrapperException(
            "Action Failed inside Intkey Wrapper::inContract Code");

    if (!executer.SetMessage(inMessage.Message.c_str(), inMessage.OriginatorID.c_str()))
    {
        throw CppContractWrapperException("Action Failed inside Intkey Wrapper::SetMessage");
    }

    if (!executer.ExecuteMessage(inContractID.c_str(), inCreatorID.c_str()))
    {
        throw CppContractWrapperException(
            "Action Failed inside Intkey Wrapper::ExecuteMessgae");
    }

    try
    {
        char outStateRaw[MIN_STATE_BUFFER_SIZE];

        if (executer.GetOutState(outStateRaw, sizeof(outStateRaw)))
        {
            outContractState.State = outStateRaw;
        }
    }
    catch (...)
    {
        throw CppContractWrapperException(
            "Action Failed inside Intkey Wrapper::Getoutof state");
    }
}

void CppContractWrapper::send_message_to_contract(const std::string& inContractID,
    const std::string& inCreatorID,
    const pc::ContractCode& inContract,
    const pc::ContractMessage& inMessage,
    const pc::ContractState& inContractState,
    pc::ContractState& outContractState,
    std::map<std::string, std::string>& outDependencies,
    std::string& outMessageResult)
{
/*    bool result = true;
    if (!executer.SetCode(inContract.Code.c_str()))
        throw CppContractWrapperException(
            "Action Failed inside Intkey Wrapper::inContract Code");

    if (!executer.SetMessage(inMessage.Message.c_str(), inMessage.OriginatorID.c_str()))
    {
        throw CppContractWrapperException("Action Failed inside Intkey Wrapper::SetMessage");
    }

    if (!(executer.SetInState(inContractState.State.c_str())))
    {
        throw CppContractWrapperException("Action Failed inside Intkey Wrapper::SetInState");
    }
    if (!executer.ExecuteMessage(inContractID.c_str(), inCreatorID.c_str()))
    {
        throw CppContractWrapperException(
            "Action Failed inside Intkey Wrapper::ExecuteMessgae");
    }

    try
    {
        result = false;
        char outStateRaw[MIN_STATE_BUFFER_SIZE];

        if (executer.GetOutState(outStateRaw, sizeof(outStateRaw)))
        {
            char outResultRaw[MIN_RESULT_BUFFER_SIZE];

            outContractState.State = outStateRaw;

            if (executer.GetResult(outResultRaw, sizeof(outResultRaw)))
            {
                outMessageResult = outResultRaw;

                // TODO: Get dependencies
                result = true;
            }
        }
    }
    catch (...)
    {
        throw CppContractWrapperException(
            "Action Failed inside Intkey Wrapper::Getoutof state");
    }

    if (!result)
        HandleFailure();


}

void CppContractWrapper::HandleFailure()
{
    // TODO: Through a proper exception defined by the PDO
    // Initially any exception should work


    throw CppContractWrapperException("Action Failed inside Intkey Wrapper");
}*/

