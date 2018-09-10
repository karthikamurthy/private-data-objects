#include "CppProcessor.h"
#include "ContractDispatcher.cpp"
#include <iostream>
#include <string>

CppContractWrapper* LookUpContract(std::string contract_code)
{
    for (int i = 0; contractDispatchTable[i].contract_id != NULL; i++)
    {
        if (contract_code.compare(contractDispatchTable[i].contract_id) == 0)
        {
            return contractDispatchTable[i].contract_factory_ptr();
        }
    }
    pdo::error::RuntimeError("Contract not  Found\n");

    return nullptr;
}

pc::ContractInterpreter* LookUpWorkOrder(std::string work_oder_code)
{
    for (int i = 0; workOrderDispatchTable[i].project_name != NULL; i++)
    {
        if (work_oder_code.compare(workOrderDispatchTable[i].project_name) == 0)
        {
            return workOrderDispatchTable[i].work_order_factory_ptr();
        }
    }
    pdo::error::RuntimeError("WorkOrder not  Found\n");
	return NULL;
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
    CppContractWrapper* executer = LookUpContract(enclave_type);

    if(!executer)
        return;

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
    std::string contractCode = inContract.Code.c_str();
    std::size_t pos = contractCode.find(':');
    std::string enclave_type = contractCode.substr(pos + 1);
    CppContractWrapper* executer = LookUpContract(enclave_type);

    if(!executer)
        return;

    bool result = true;
    if (!executer->SetCode(inContract.Code.c_str()))
        executer->HandleFailure("Set contract code");

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

void CppProcessor::process_work_order(
	std::string code_id,
	ByteArray tc_service_address,
	ByteArray participant_address,
	ByteArray enclave_id,
	ByteArray work_order_id,
	std::vector<pdo::WorkOrderData>& work_order_data)
{
	pdo::contracts::ContractInterpreter* work_order = LookUpWorkOrder(code_id);
	work_order->process_work_order(
		code_id,
		tc_service_address,
		participant_address,
		enclave_id,
		work_order_id,
		work_order_data);
}
