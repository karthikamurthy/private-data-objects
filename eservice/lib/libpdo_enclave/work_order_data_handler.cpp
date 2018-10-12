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
#include "skenc.h"
#include "jsonvalue.h"
#include "parson.h"

#include "enclave_utils.h"
#include "enclave_data.h"

#include "interpreter/ContractInterpreter.h"
#include "work_order_processor.h"
#include "work_order_data_handler.h"



// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
namespace pdo
{
	void WorkOrderDataHandler::Unpack(EnclaveData& enclaveData, const JSON_Object* object)
	{
		ByteArray encrypted_input_data;
		ByteArray input_data_hash;

		data_type = WorkOrderProcessor::GetJsonStr(
			object,
			"Type",
			"failed to retrieve work order data type");

		output_link = WorkOrderProcessor::GetJsonStr(object, "OutputLink");

		//WorkOrderProcessor::GetByteArray(
		//	object,
		//	"BLOB",
		//	NULL, //"failed to retrieve work order input data",
	    //	encrypted_input_data);

        //ByteArray encrypted_data_encryption_key;
        //WorkOrderProcessor::GetByteArray(
		//	object,
		//	"EncryptedDataEncryptionKey",
		//	"failed to retrieve work order data encryption key",
		//	encrypted_data_encryption_key);

        std::string enc_data_key_str = WorkOrderProcessor::GetJsonStr(object, "EncryptedDataEncryptionKey");
        if (!enc_data_key_str.empty())
        {
            ByteArray enc_data_key = Base64EncodedStringToByteArray(enc_data_key_str);
            // ??? data_encryption_key = rsa_decrypt_message_ptr(enc_data_key);
            data_encryption_key = enclaveData.decrypt_message(enc_data_key);
        }
        else
        {
            data_encryption_key.clear();
        }

 		std::string data_b64 = WorkOrderProcessor::GetJsonStr(object, "BLOB");
        if (!data_b64.empty())
        {
            if (!data_encryption_key.empty())
            {
                encrypted_input_data = Base64EncodedStringToByteArray(data_b64);
            }
            else
            {
                encrypted_input_data.assign(data_b64.begin(), data_b64.end());
            }
        }
        else
        {
            encrypted_input_data.clear();
        }
        data_b64.clear();

    	if (!encrypted_input_data.empty())
		{
            DecryptInputData(encrypted_input_data);

            WorkOrderProcessor::GetByteArray(
				object,
				"Sha256Hash",
				NULL, //"failed to retrieve work order input hash",
				input_data_hash);

			if (!input_data_hash.empty())
			{
			    VerifyInputHash(decrypted_input_data, input_data_hash);
			}
		}
	}

	void WorkOrderDataHandler::Pack(JSON_Array* json_array)
	{
		JSON_Status jret;
		ComputeOutputHash();

		JSON_Value* data_item_value = json_value_init_object();
		pdo::error::ThrowIfNull(data_item_value, "failed to create a data item");

		JSON_Object* data_item_object = json_value_get_object(data_item_value);
		pdo::error::ThrowIfNull(data_item_object, "failed to create a data item");

		WorkOrderProcessor::JsonSetStr(
			data_item_object,
			"Type",
			data_type.c_str(),
			"failed to serialize data type");

		std::string output_hash_str(output_hash.begin(), output_hash.end());
		WorkOrderProcessor::JsonSetStr(
			data_item_object,
			"Sha256Hash",
			output_hash_str.c_str(),
			"failed to serialize data type");

		WorkOrderProcessor::JsonSetStr(
			data_item_object,
			"OutputLink",
			output_link.c_str(),
			"failed to serialize data output link");

		std::string encrypted_output_str = EncryptOutputData();
		WorkOrderProcessor::JsonSetStr(
			data_item_object,
			"BLOB",
			encrypted_output_str.c_str(),
			"failed to serialize encrypted output data");

		jret = json_array_append_value(json_array, data_item_value);
		pdo::error::ThrowIf<pdo::error::RuntimeError>(
			jret != JSONSuccess, "failed to add item to the data array");
	}

	void WorkOrderDataHandler::ComputeOutputHash()
	{
		// dummy for phase 1 and 2
		std::string str = "d111";
		std::copy(str.begin(), str.end(), std::back_inserter(output_hash));
	}

	void WorkOrderDataHandler::VerifyInputHash(ByteArray input_data, ByteArray input_hash)
	{
		// do nothing at the phase 1
	}

	void WorkOrderDataHandler::DecryptInputData(ByteArray encrypted_input_data)
	{

	    if (data_encryption_key.size() > 0)
	    {
            decrypted_input_data = pdo::crypto::skenc::DecryptMessage(data_encryption_key, encrypted_input_data);
	    }
        else
        {
		    decrypted_input_data = encrypted_input_data;
		}
	}

	std::string WorkOrderDataHandler::EncryptOutputData()
	{
	    std::string encrypted_output_str;
	    if (decrypted_output_data.empty())
	    {
	        encrypted_output_str.clear();
	    }
	    else
	    if (data_encryption_key.size() > 0)
	    {
	        ByteArray enc_data = pdo::crypto::skenc::EncryptMessage(data_encryption_key, decrypted_output_data);
	        encrypted_output_str = ByteArrayToBase64EncodedString(enc_data);
	    }
        else
        {
       		encrypted_output_str.assign(decrypted_output_data.begin(), decrypted_output_data.end());
		}
		return encrypted_output_str;
	}
}
