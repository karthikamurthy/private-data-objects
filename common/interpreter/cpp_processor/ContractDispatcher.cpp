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

#include <map>
#include <string>
#include "work_order.h"
#include "work_order_data.h"
#include "CppProcessor.h"
#include "echo_work_order/EchoWorkOrder.h"

CppContractWrapper* intkey_factory();
CppContractWrapper* echo_factory();
pdo::contracts::ContractInterpreter* echo_result_factory();

class EchoResult: public pdo::contracts::ContractInterpreterBase
{
public:
	void process_work_order(
		std::string code_id,
		ByteArray tc_service_address,
		ByteArray participant_address,
		ByteArray enclave_id,
		ByteArray work_order_id,
		std::vector<pdo::WorkOrderData>& work_order_data)
	{
		std::string result_str;
		pdo::WorkOrderData* message = 0;
		pdo::WorkOrderData* result = 0;
		for (size_t i = 0; i < work_order_data.size(); i++)
		{
			pdo::WorkOrderData& wo_data = work_order_data.at(i);
			if (wo_data.data_type == "message")
			{
				message = &wo_data;
			}
			else
			if (wo_data.data_type == "result")
			{
				result = &wo_data;
			}
		}
		pdo::error::ThrowIfNull(message, "work order message not found");
		pdo::error::ThrowIfNull(result, "work order result not found");

		EchoResultImpl echo_result_impl;
		result_str = echo_result_impl.Process(
			pdo::WorkOrder::ByteArrayToStr(message->decrypted_input_data));
		
		result->decrypted_output_data = pdo::WorkOrder::StrToByteArray(result_str);
	};

} echo_result;

pdo::contracts::ContractInterpreter* echo_result_factory()
{
	return &echo_result;
}

ContractDispatchTableEntry contractDispatchTable[] = {
	{"intkey:", intkey_factory},
    { "echo:", echo_factory },
    {NULL, NULL}
};

WorkOrderDispatchTableEntry workOrderDispatchTable[] = {
	{"echo-result:", echo_result_factory},
    {NULL, NULL}
};
