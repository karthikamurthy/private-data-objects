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

#include <string>
#include <map>

#include "ContractState.h"
#include "ContractCode.h"
#include "ContractMessage.h"
#include "work_order_data.h"
#include "error.h"

namespace pdo
{
    namespace contracts
    {
        // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
        class ContractInterpreter
        {
        public:

            virtual void create_initial_contract_state(
                const std::string& inContractID,
                const std::string& inCreatorID,
                const ContractCode& inContract,
                const ContractMessage& inMessage,
                ContractState& outContractState
                ) = 0;

            virtual void send_message_to_contract(
                const std::string& inContractID,
                const std::string& inCreatorID,
                const ContractCode& inContract,
                const ContractMessage& inMessage,
                const ContractState& inContractState,
                ContractState& outContractState,
                std::map<std::string,std::string>& outDependencies,
                std::string& outMessageResult
                ) = 0;

            virtual void process_work_order(
				std::string code_id,
				ByteArray tc_service_address,
				ByteArray participant_address,
				ByteArray enclave_id,
				ByteArray work_order_id,
				std::vector<pdo::WorkOrderData>& work_order_data)
            {
                pdo::error::RuntimeError("Process work order not implemented\n");
            }
        };

        class ContractInterpreterBase: public ContractInterpreter
		{
		public:

			virtual void create_initial_contract_state(
				const std::string& inContractID,
				const std::string& inCreatorID,
				const ContractCode& inContract,
				const ContractMessage& inMessage,
				ContractState& outContractState
				)
			{
                pdo::error::RuntimeError("create_initial_contract_state not implemented\n");
			};

			virtual void send_message_to_contract(
				const std::string& inContractID,
				const std::string& inCreatorID,
				const ContractCode& inContract,
				const ContractMessage& inMessage,
				const ContractState& inContractState,
				ContractState& outContractState,
				std::map<std::string, std::string>& outDependencies,
				std::string& outMessageResult
				)
			{
                pdo::error::RuntimeError("send_message_to_contract not implemented\n");
			};

			virtual void process_work_order(
				std::string code_id,
				ByteArray tc_service_address,
				ByteArray participant_address,
				ByteArray enclave_id,
				ByteArray work_order_id,
				std::vector<pdo::WorkOrderData>& work_order_data)
			{
                pdo::error::RuntimeError("process_work_order not implemented\n");
			}
		};

    }

}

