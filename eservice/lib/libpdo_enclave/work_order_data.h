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


namespace pdo
{
	class WorkOrderData
	{
	public:
		WorkOrderData(){};

		void Unpack(const JSON_Object* object);
		void Pack(JSON_Array* json_array);

		std::string data_type;
		std::string output_link;
		ByteArray decrypted_input_data;
		ByteArray decrypted_output_data;

		ByteArray input_hash;
		ByteArray output_hash;

	protected:
		void ComputeOutputHash();
		void VerifyInputHash(ByteArray input_data, ByteArray input_hash);
		void DecryptInputData(ByteArray encrypted_input_data);
		std::string EncryptOutputData();

		ByteArray data_encryption_key;
	};
}
