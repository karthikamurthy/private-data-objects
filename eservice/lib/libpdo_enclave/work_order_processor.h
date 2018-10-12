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
#include <vector>

#include "enclave_data.h"

#include "parson.h"
#include "types.h"
#include "work_order_data_handler.h"

namespace pdo
{
	class WorkOrderProcessor
	{
	public:
		WorkOrderProcessor(){};

		ByteArray CreateErrorResponse(int err_code, const char* err_message);

		ByteArray Process(EnclaveData& enclaveData, std::string json_str);

		static const char* GetJsonStr(
			const JSON_Object* json_object,
			const char* name,
			const char* err_msg = NULL);

		static double GetJsonNumber(const JSON_Object* object, const char* name);

		static void GetByteArray(const JSON_Object* object,
			const char* name,
			const char* err_msg,
			ByteArray& dst);

		static void JsonSetStr(
			JSON_Object* json,
			const char* name,
			const char* value,
			const char* err_msg);

		static void JsonSetNumber(
			JSON_Object* json,
			const char* name,
			double value,
			const char* err_msg);

		static ByteArray StrToByteArray(std::string str);
		static std::string ByteArrayToStr(ByteArray ba);

	protected:
		void ParseJsonInput(EnclaveData& enclaveData, std::string);
		ByteArray CreateJsonOutput(std::vector<pdo::WorkOrderData> wo_data);
		std::vector<pdo::WorkOrderData> ExecuteWorkOrder();
		void VerifySignature();
		void ComputeSignature();

		std::vector<WorkOrderDataHandler> data_items;

		std::string participant_signature;
		std::string participant_generated_nonce;
		std::string tc_service_address;
		std::string enclave_id;
		std::string work_order_id;
		std::string participant_address;
		double json_request_id;

		std::string enclave_generated_nonce;
		std::string enclave_signature;
	};
}
