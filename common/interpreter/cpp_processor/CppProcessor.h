/* Copyright 2018 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#pragma once

#include <map>
#include <string>
#include "ContractInterpreter.h"
#include "CppProcessorHandler.h"

namespace pc = pdo::contracts;

typedef CppContractWrapper* (*contract_factory)();
typedef pc::ContractInterpreter* (*work_order_factory)();

struct ContractDispatchTableEntry
{
    const char* contract_id;
    contract_factory contract_factory_ptr;
};

struct WorkOrderDispatchTableEntry
{
    const char* project_name;
    work_order_factory work_order_factory_ptr;
};

class CppProcessor : public pc::ContractInterpreter
{
public:
    CppProcessor(void);
    ~CppProcessor(void);

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

    virtual void process_work_order(
		std::string code_id,
		ByteArray tc_service_address,
		ByteArray participant_address,
		ByteArray enclave_id,
		ByteArray work_order_id,
		std::vector<pdo::WorkOrderData>& work_order_data);
};
