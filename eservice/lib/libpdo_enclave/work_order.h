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

#include "parson.h"

#include "types.h"
#include "work_order_data.h"

class WorkOrder
{
public:
	WorkOrder(){};
	
	ByteArray CreateErrorResponse(int err_code, const char* err_message);

	ByteArray Process(std::string json_str);

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

protected:
	void ParseJsonInput(std::string);
	ByteArray CreateJsonOutput();
	void ExecuteWorkOrder();
	void VerifySignature();
	void ComputeSignature();
	
	std::vector<pdo::WorkOrderData> data_items;
	
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
