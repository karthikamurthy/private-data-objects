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

#include "EchoCppContractExecuter.h"
#include <stdio.h>
#include <iostream>

static int StrLen(char* str);
static char* StrCpy(const char* src, char* dst, int size);
static char* UintToStr(unsigned int value, char* buf, int size);
static const char* StrToUint(
const char* strPtr, unsigned int* ptrVal, const char* terminators = ",");

CppContractWrapper* echo_factory()
{
    return new EchoCppContractExecuter();
}

int EchoCode::Init(const char* str)
{
    int result = STUB_INTERPRETOR_ERR_CODE;
    if (str != NULL)
    {
       return STUB_INTERPRETOR_NO_ERROR;
    }
     return STUB_INTERPRETOR_ERR_CODE;
}

int EchoMessage::Init(const char* str)
{
    int result = STUB_INTERPRETOR_ERR_MESSAGE;
    try
    {
        if (str != NULL)
        {
                if (StrToUint(str, &value) != NULL)
                {
                    result = STUB_INTERPRETOR_NO_ERROR;
                }
        }
    }
    catch (...)
    {
        return STUB_INTERPRETOR_ERR_MESSAGE;
    }
    return result;
}

int EchoState::Init(const char* str)
{
    int result = STUB_INTERPRETOR_ERR_STATE;
    try
    {
        if (str == NULL)
        {
            terminated = 0;
            value = 0;
            result = STUB_INTERPRETOR_NO_ERROR;
        }
        else if ((str = StrToUint(str, &terminated)) != NULL)
        {
            str++;
            if (StrToUint(str, &value) != NULL)
            {
                result = STUB_INTERPRETOR_NO_ERROR;
            }
        }
    }
    catch (...)
    {
        return STUB_INTERPRETOR_ERR_STATE;
    }

    return result;
}

int EchoState::Serialize(char* buf, int bufSize)
{
    int result = STUB_INTERPRETOR_NO_ERROR;

    if (bufSize < MIN_STATE_BUFFER_SIZE)
    {
        result = STUB_INTERPRETOR_ERR_STATE;
    }
    else
    {
        *buf++ = !terminated ? '0' : '1';
        *buf++ = ',';
        UintToStr(value, buf, bufSize - 2);
    }
    return result;
}

bool EchoCppContractExecuter::ExecuteMessage(const char* contractId, const char* creatorId)
{
    // TODO: contractId and creatorId are not used

    if (result == STUB_INTERPRETOR_NO_ERROR)
    {
        state.value = message.value;

        if (state.terminated)
        {
            result = STUB_INTERPRETOR_ERR_TERMINATED;
        }
    }

    return (result == STUB_INTERPRETOR_NO_ERROR);
}

bool EchoCppContractExecuter::GetResult(char* buf, int bufSize)
{
    if (bufSize < MIN_RESULT_BUFFER_SIZE || result != STUB_INTERPRETOR_NO_ERROR)
    {
        return false;
    }

    if (!UintToStr(state.value, buf, bufSize))
    {
        return false;
    }

    return true;
}

void EchoCppContractExecuter::HandleFailure(const char* msg)
{
    throw EchoCppContractException(msg);
}

const char* StrToUint(const char* strPtr, unsigned int* ptrVal, const char* terminators)
{
    *ptrVal = 0;
    while (*strPtr >= '0' && *strPtr <= '9')
    {
        *ptrVal = (*ptrVal * 10) + (*strPtr - '0');
        strPtr++;
    }

    if (*strPtr && terminators != NULL)
    {
        while (*terminators)
        {
            if (*strPtr == *terminators++)
            {
                return strPtr;
            }
        }
        return NULL;
    }
    return strPtr;
}

char* UintToStr(unsigned int value, char* buf, int size)
{
    if (buf && size > 1)
    {
        if (!value)
        {
            *buf++ = '0';
            *buf = 0;
        }
        else
        {
            char* ptr = buf;
            unsigned int temp = value;

            while (temp)
            {
                ptr++;
                temp = temp / 10;
            }

            if ((ptr - buf) < size)
            {
                buf = ptr;
                *ptr-- = 0;
                while (value != 0)
                {
                    *ptr-- = value % 10 + '0';
                    value = value / 10;
                }
            }
        }
    }

    return buf;
}

int StrLen(char* str)
{
    int len = 0;

    if (str)
    {
        while (*str++)
        {
            len++;
        }
    }
    return len;
}

char* StrCpy(const char* src, char* dst, int size)
{
    if (src && dst && size)
    {
        while (--size && *src)
        {
            *dst++ = *src++;
        }
        *dst = 0;
    }
    return dst;
}
