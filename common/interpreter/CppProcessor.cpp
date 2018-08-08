#include "CppProcessor.h"
#include <iostream>
#include <string>

extern ContractDispatchTableEntry contractDispatchTable[];

ContractDispatchTableEntry* LookUpContract(std::string contract_code)
{
    for (int i = 0; contractDispatchTable[i].project_name != NULL; i++)
    {
        int l = strlen(contractDispatchTable[i].project_name);
        if (contract_code.compare(contractDispatchTable[i].project_name) == 0)
        {
            return &contractDispatchTable[i];
        }
    }
    // if we are here, the contract is not found -> throw an exception
}

CppProcessor::CppProcessor() {}

CppProcessor::~CppProcessor() {}

void CppProcessor::create_initial_contract_state(const std::string& inContractID,
    const std::string& inCreatorID,
    const pc::ContractCode& inContract,
    const pc::ContractMessage& inMessage,
    pc::ContractState& outContractState)
{
    std::string contractCode = inContract.Code.c_str();
    std::size_t pos = contractCode.find(':');
    std::string enclave_type = contractCode.substr(pos + 1);
    CppContractWrapper* executer = LookUpContract(enclave_type)->contract_factory_ptr();

    //entry->contract_factory_ptr()->create_initial_contract_state(
    //    inContractID, inCreatorID, inContract, inMessage, outContractState);
    if (!executer->SetCode(inContract.Code.c_str()))
         executer->HandleFailure("Set contract code");

    if (!executer->SetMessage(inMessage.Message.c_str(), inMessage.OriginatorID.c_str()))
    {
         executer->HandleFailure("Set contract Message");
    }

    if (!executer->ExecuteMessage(inContractID.c_str(), inCreatorID.c_str()))
    {
        executer->HandleFailure("Execute Message");
    }

    try
    {
        char outStateRaw[MIN_STATE_BUFFER_SIZE];

        if (executer->GetOutState(outStateRaw, sizeof(outStateRaw)))
        {
            outContractState.State = outStateRaw;
        }
    }
    catch (...)
    {
        executer->HandleFailure("unknown error");
    }
}

void CppProcessor::send_message_to_contract(const std::string& inContractID,
    const std::string& inCreatorID,
    const pc::ContractCode& inContract,
    const pc::ContractMessage& inMessage,
    const pc::ContractState& inContractState,
    pc::ContractState& outContractState,
    std::map<std::string, std::string>& outDependencies,
    std::string& outMessageResult)
{
    //ContractDispatchTableEntry* entry = LookUpContract(inContract.Code.c_str());
    //entry->contract_factory_ptr()->send_message_to_contract(inContractID, inCreatorID, inContract,
    //    inMessage, inContractState, outContractState, outDependencies, outMessageResult);
    CppContractWrapper* executer = LookUpContract(inContract.Code.c_str())->contract_factory_ptr();

    bool result = true;
    if (!executer->SetCode(inContract.Code.c_str()))
        executer->HandleFailure("Set contract code");
        //throw CppContractWrapperException(
        //    "Action Failed inside Intkey Wrapper::inContract Code");

    if (!executer->SetMessage(inMessage.Message.c_str(), inMessage.OriginatorID.c_str()))
    {
        executer->HandleFailure("SetMessage");
    }

    if (!(executer->SetInState(inContractState.State.c_str())))
    {
        executer->HandleFailure("SetInState");
    }
    if (!executer->ExecuteMessage(inContractID.c_str(), inCreatorID.c_str()))
    {
        executer->HandleFailure("ExecuteMessgae");
    }

    try
    {
        result = false;
        char outStateRaw[MIN_STATE_BUFFER_SIZE];

        if (executer->GetOutState(outStateRaw, sizeof(outStateRaw)))
        {
            char outResultRaw[MIN_RESULT_BUFFER_SIZE];

            outContractState.State = outStateRaw;

            if (executer->GetResult(outResultRaw, sizeof(outResultRaw)))
            {
                outMessageResult = outResultRaw;

                // TODO: Get dependencies
                result = true;
            }
        }
    }
    catch (...)
    {
        executer->HandleFailure("Getoutof state");
    }

    if (!result)
        executer->HandleFailure("Unknown error");
}
