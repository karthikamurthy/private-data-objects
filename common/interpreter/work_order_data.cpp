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

#include "crypto/crypto.h"
#include "crypto/skenc.h"
#include "jsonvalue.h"
#include "packages/parson/parson.h"

#include "interpreter/ContractInterpreter.h"
#include "work_order.h"
#include "work_order_data.h"

namespace pdo
{
	void WorkOrderData::Unpack(const JSON_Object* object)
	{
		ByteArray encrypted_input_data;
		ByteArray input_data_hash;

		data_type = WorkOrder::GetJsonStr(
			object,
			"Type",
			"failed to retrieve work order data type");

		output_link = WorkOrder::GetJsonStr(object, "OutputLink");

		WorkOrder::GetByteArray(
			object,
			"BLOB",
			NULL, //"failed to retrieve work order input data",
			encrypted_input_data);

        WorkOrder::GetByteArray(
			object,
			"EncryptedDataEncryptionKey",
			"failed to retrieve work order data encryption key",
			data_encryption_key);

		if (!encrypted_input_data.empty())
		{
            DecryptInputData(encrypted_input_data);

            WorkOrder::GetByteArray(
				object,
				"Sha256Hash",
				"failed to retrieve work order input hash",
				input_data_hash);

			VerifyInputHash(decrypted_input_data, input_data_hash);
		}
	}

	void WorkOrderData::Pack(JSON_Array* json_array)
	{
		JSON_Status jret;
		ComputeOutputHash();

		JSON_Value* data_item_value = json_value_init_object();
		pdo::error::ThrowIfNull(data_item_value, "failed to create a data item");

		JSON_Object* data_item_object = json_value_get_object(data_item_value);
		pdo::error::ThrowIfNull(data_item_object, "failed to create a data item");

		WorkOrder::JsonSetStr(
			data_item_object,
			"Type",
			data_type.c_str(),
			"failed to serialize data type");

		std::string output_hash_str(output_hash.begin(), output_hash.end());
		WorkOrder::JsonSetStr(
			data_item_object,
			"Sha256Hash",
			output_hash_str.c_str(),
			"failed to serialize data type");

		WorkOrder::JsonSetStr(
			data_item_object,
			"OutputLink",
			output_link.c_str(),
			"failed to serialize data output link");

		std::string encrypted_output_str = EncryptOutputData();
		WorkOrder::JsonSetStr(
			data_item_object,
			"BLOB",
			encrypted_output_str.c_str(),
			"failed to serialize encrypted output data");

		jret = json_array_append_value(json_array, data_item_value);
		pdo::error::ThrowIf<pdo::error::RuntimeError>(
			jret != JSONSuccess, "failed to add item to the data array");
	}

	void WorkOrderData::ComputeOutputHash()
	{
		// dummy for phase 1 and 2
		std::string str = "d111";
		std::copy(str.begin(), str.end(), std::back_inserter(output_hash));
	}

	void WorkOrderData::VerifyInputHash(ByteArray input_data, ByteArray input_hash)
	{
		// do nothing at the phase 1
	}

	void WorkOrderData::DecryptInputData(ByteArray encrypted_input_data)
	{
		decrypted_input_data = pdo::crypto::skenc::DecryptMessage(data_encryption_key, encrypted_input_data);
		//decrypted_input_data = encrypted_input_data;
	}

	std::string WorkOrderData::EncryptOutputData()
	{
		// no actual encryption in the phases 1 and 2
		// just copy byte array to a string
        ByteArray encrypted_output_data =
            pdo::crypto::skenc::EncryptMessage(data_encryption_key, decrypted_output_data);
		std::string encrypted_output_str(
			encrypted_output_data.begin(),
			encrypted_output_data.end());
		return encrypted_output_str;
	}
}
