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
#define STUB_INTERPRETOR_NO_ERROR (0)
#define STUB_INTERPRETOR_ERR (1)
#define STUB_INTERPRETOR_ERR_CODE (2)
#define STUB_INTERPRETOR_ERR_MESSAGE (3)
#define STUB_INTERPRETOR_ERR_STATE (4)
#define STUB_INTERPRETOR_ERR_PARAM (5)
#define STUB_INTERPRETOR_ERR_TERMINATED (6)
#define STUB_INTERPRETOR_ERR_RESULT (7)
#define STUB_INTERPRETOR_ERR_STRING_NULL (8)
#define STUB_INTERPRETOR_ERR_STRING_TO_INT (9)

#include <map>
#include <string>
#include <memory>

#define MIN_RESULT_BUFFER_SIZE (100)
#define MIN_STATE_BUFFER_SIZE (13)

struct ErrorInfo
{
    int code;
    const char* message;
};

class CppContractWrapper; // Forward declaration

CppContractWrapper* intkey_factory();
CppContractWrapper* echo_factory();

class CppContractWrapperException : public std::exception
{
public:
    CppContractWrapperException(const char* msg) : msg_(msg) {}
    virtual char const* what() const noexcept { return msg_.c_str(); }

protected:
    std::string msg_;
};

class CppContractWrapper
{
public:
    CppContractWrapper(){}

    virtual bool SetCode(const char* codeStr) = 0;
    virtual bool SetMessage(const char* messageStr, const char* originatorId) = 0;
    virtual bool SetInState(const char* stateStr) = 0;
    virtual bool ExecuteMessage(const char* contractId, const char* creatorId) = 0;
    virtual bool GetResult(char* buf, int bufSize) = 0;
    virtual bool GetOutState(char* buf, int bufSize) = 0;
    virtual void HandleFailure(const char* msg) = 0;

    virtual ~CppContractWrapper() {
    }
};
