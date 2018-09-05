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

#include <algorithm>
#include <string>
#include <vector>

#include "error.h"
#include "pdo_error.h"
#include "types.h"

#include "crypto.h"
#include "jsonvalue.h"
#include "parson.h"

#include "enclave_utils.h"
#include "interpreter/ContractInterpreter.h"

#include "work_order.h"
#include "work_order_data.h"

const char* WorkOrder::GetJsonStr(
	const JSON_Object* json_object, 
	const char* name, 
	const char* err_msg)
{
	const char* pvalue = json_object_dotget_string(json_object, name);
	
	if (!pvalue)
	{
		if (err_msg)
		{
			pdo::error::ThrowIf<pdo::error::ValueError>(!pvalue, err_msg);
		}
		else
		{
			pvalue = "";
		}
	}
	return pvalue;
}

void WorkOrder::GetByteArray(const JSON_Object* object,
	const char* name,
	const char* err_msg,
	ByteArray& dst)
{
	const char* pvalue = WorkOrder::GetJsonStr(object, name, err_msg);
	if (!pvalue)
	{
		if (err_msg)
		{
			pdo::error::ThrowIf<pdo::error::ValueError>(!pvalue, err_msg);
		}
		else
		{
			pvalue = "";
		}
	}

	std::string str = pvalue;
	std::copy(str.begin(), str.end(), std::back_inserter(dst));
}

double WorkOrder::GetJsonNumber(const JSON_Object* object, const char* name)
{
	return json_object_dotget_number(object, name);
}

void WorkOrder::ParseJsonInput(std::string json_str)
{
	// Parse the work order request
	JsonValue parsed(json_parse_string(json_str.c_str()));
	pdo::error::ThrowIfNull(
		parsed.value, "failed to parse the work order request, badly formed JSON");

	JSON_Object* request_object = json_value_get_object(parsed);
	pdo::error::ThrowIfNull(request_object, "Missing JSON object in work order request");

	json_request_id = GetJsonNumber(request_object, "id");

	JSON_Object* params_object = json_object_dotget_object(request_object, "params");
	pdo::error::ThrowIfNull(params_object, "Missing params object in work order request");

	work_order_id = GetJsonStr(
		params_object, 
		"WorkOrderId", 
		"invalid request; failed to retrieve work order id");

	participant_signature = GetJsonStr(
		params_object,
		"ParticipantSignature",
		"invalid request; failed to retrieve participant signature");

	participant_generated_nonce = GetJsonStr(
		params_object,
		"ParticipantGeneratedNonce",
		"invalid request; failed to retrieve participant generated nonce");
	
	tc_service_address = GetJsonStr(
		params_object,
		"TcServiceAddress",
		"invalid request; failed to retrieve TC service address");

	enclave_id = GetJsonStr(
		params_object,
		"EnclaveId",
		"invalid request; failed to retrieve enclave id");

	participant_address = GetJsonStr(
		params_object,
		"ParticipantAddress",
		"invalid request; failed to retrieve participant address");

	JSON_Array* data_array = json_object_get_array(params_object, "Data");

	size_t count = json_array_get_count(data_array);
	for (size_t i = 0; i < count; i++)
	{
		JSON_Object* data_object = json_array_get_object(data_array, i);
		pdo::WorkOrderData wo_data;
		wo_data.Unpack(data_object);
		data_items.push_back(wo_data);
	}
}

void WorkOrder::JsonSetStr(JSON_Object* json, const char* name, const char* value, const char* err)
{
	JSON_Status jret = json_object_dotset_string(json, name, value);
	pdo::error::ThrowIf<pdo::error::RuntimeError>(jret != JSONSuccess, err);
}

void WorkOrder::JsonSetNumber(JSON_Object* json, const char* name, double value, const char* err)
{
	JSON_Status jret = json_object_dotset_number(json, name, value);
	pdo::error::ThrowIf<pdo::error::RuntimeError>(jret != JSONSuccess, err);
}

ByteArray WorkOrder::CreateJsonOutput()
{
	JSON_Status jret;

	// Create the response structure
	JsonValue resp_value(json_value_init_object());
	pdo::error::ThrowIf<pdo::error::RuntimeError>(
		!resp_value.value, "Failed to create the response object");

	JSON_Object* resp = json_value_get_object(resp_value);
	pdo::error::ThrowIfNull(resp, "Failed on retrieval of response object value");

	JsonSetStr(resp, "jsonrpc", "2.0", "failed to serialize jsonrpc");
	JsonSetNumber(resp, "id", json_request_id, "failed to serialize json request id");

	jret = json_object_set_value(resp, "result", json_value_init_object());
	pdo::error::ThrowIf<pdo::error::RuntimeError>(
		jret != JSONSuccess, "failed to serialize result");

	JSON_Object* result = json_object_get_object(resp, "result");
	pdo::error::ThrowIfNull(result, "failed to serialize the result");

	JsonSetStr(result, "WorkOrderId", work_order_id.c_str(), "failed to serialize work order id");

	jret = json_object_set_value(result, "Data", json_value_init_array());
	pdo::error::ThrowIf<pdo::error::RuntimeError>(
		jret != JSONSuccess, "failed to serialize the result data");

	JSON_Array* data_array = json_object_get_array(result, "Data");
	pdo::error::ThrowIfNull(data_array, "failed to serialize the dependency array");

	for( pdo::WorkOrderData d : this->data_items)
	{
		if (!d.decrypted_output_data.empty())
		{
			d.Pack(data_array);
		}
	}

	ComputeSignature();

	JsonSetStr(
		result,
		"EnclaveGeneratedNonce",
		enclave_generated_nonce.c_str(),
		"failed to serialize enclave generated nonce");

	JsonSetStr(
		result,
		"EnclaveSignature",
		enclave_signature.c_str(),
		"failed to serialize enclave signature");

	// serialize the resulting json
	size_t serializedSize = json_serialization_size(resp_value);
	ByteArray serialized_response;
	serialized_response.resize(serializedSize);

	jret = json_serialize_to_buffer(resp_value,
		reinterpret_cast<char*>(&serialized_response[0]), serialized_response.size());
	pdo::error::ThrowIf<pdo::error::RuntimeError>(
		jret != JSONSuccess, "contract response serialization failed");

	return serialized_response;
}

void WorkOrder::ExecuteWorkOrder()
{
	// dummy processing for phase 1
	//for each (WorkOrderData& wo_data in data_items)
	//{
	for (size_t i = 0; i < data_items.size(); i++)
	{
		pdo::WorkOrderData& wo_data = data_items.at(i);
		if (wo_data.data_type == "result")
		{
			std::string str = "dummy output";
			
			//for each (char ch in str)
			//{
				//wo_data.decrypted_output_data.push_back(ch);
			//}
			std::copy(str.begin(),str.end(),std::back_inserter(wo_data.decrypted_output_data));
			break;
		}
	}
}

void WorkOrder::VerifySignature()
{
	// do nothing during phases 1 and 2
}

void WorkOrder::ComputeSignature()
{
	// dummy implementation for phases phases 1 and 2
	enclave_generated_nonce = "123";
	enclave_signature = "99999";
}

ByteArray WorkOrder::CreateErrorResponse(int err_code, const char* err_message)
{
	JSON_Status jret;

	// Create the response structure
	JsonValue resp_value(json_value_init_object());
	pdo::error::ThrowIf<pdo::error::RuntimeError>(
		!resp_value.value, "Failed to create the response object");

	JSON_Object* resp = json_value_get_object(resp_value);
	pdo::error::ThrowIfNull(resp, "Failed on retrieval of response object value");

	JsonSetStr(resp, "jsonrpc", "2.0", "failed to serialize jsonrpc");
	JsonSetNumber(resp, "id", json_request_id, "failed to serialize json request id");

	jret = json_object_set_value(resp, "error", json_value_init_object());
	pdo::error::ThrowIf<pdo::error::RuntimeError>(
		jret != JSONSuccess, "failed to serialize error");

	JSON_Object* error = json_object_get_object(resp, "error");
	pdo::error::ThrowIfNull(error, "failed to serialize the result");

	JsonSetNumber(error, "code", err_code, "failed to serialize error code");
	JsonSetStr(error, "message", err_message, "failed to serialize error message");

	// serialize the resulting json
	size_t serializedSize = json_serialization_size(resp_value);
	ByteArray serialized_response;
	serialized_response.resize(serializedSize);

	jret = json_serialize_to_buffer(resp_value,
		reinterpret_cast<char*>(&serialized_response[0]), serialized_response.size());
	pdo::error::ThrowIf<pdo::error::RuntimeError>(
		jret != JSONSuccess, "contract response serialization failed");

	return serialized_response;

}

ByteArray WorkOrder::Process(std::string json_str)
{
	try
	{
		ParseJsonInput(json_str);
		VerifySignature();
		ExecuteWorkOrder();
		ComputeSignature();
		return CreateJsonOutput();
	}
	catch (pdo::error::ValueError& e)
	{
		SAFE_LOG(PDO_LOG_ERROR, "failed work order execution: %s", e.what());
		return CreateErrorResponse(e.error_code(), e.what());
	}
	catch (pdo::error::Error& e)
	{
		SAFE_LOG(PDO_LOG_ERROR, "exception while processing work order: %s", e.what());
		return CreateErrorResponse(e.error_code(), e.what());
	}
	catch (...)
	{
		SAFE_LOG(PDO_LOG_ERROR, "unknown exception while processing update request");
		return CreateErrorResponse(PDO_ERR_UNKNOWN, "unknown internal error");
	}
}
