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

%module pdo_enclave_internal

%include <std_vector.i>
%include <std_map.i>
%include <std_string.i>

namespace std {
    %template(StringVector) vector<string>;
    %template(StringMap) map<string, string>;
}

%include <exception.i>

%exception {
    try
    {
        $function
    }
    catch (pdo::error::MemoryError& e)
    {
        SWIG_exception(SWIG_MemoryError, e.what());
    }
    catch (pdo::error::IOError& e)
    {
        SWIG_exception(SWIG_IOError, e.what());
    }
    catch (pdo::error::RuntimeError& e)
    {
        SWIG_exception(SWIG_ValueError, e.what());
    }
    catch (pdo::error::IndexError& e)
    {
        SWIG_exception(SWIG_ValueError, e.what());
    }
    catch (pdo::error::DivisionByZero& e)
    {
        SWIG_exception(SWIG_DivisionByZero, e.what());
    }
    catch (pdo::error::OverflowError& e)
    {
        SWIG_exception(SWIG_OverflowError, e.what());
    }
    catch (pdo::error::ValueError& e)
    {
        SWIG_exception(SWIG_ValueError, e.what());
    }
    catch (pdo::error::SystemError& e)
    {
        SWIG_exception(SWIG_SystemError, e.what());
    }
    catch (pdo::error::SystemBusyError& e)
    {
        SWIG_exception(SWIG_SystemError, e.what());
    }
    catch (pdo::error::UnknownError& e) {
        SWIG_exception(SWIG_UnknownError, e.what());
    }
    catch (...)
    {
        SWIG_exception(SWIG_RuntimeError,"Unknown exception");
    }
}

%{
#include "swig_utils.h"
%}

%{
#include "pdo_enclave.h"
%}

%include "signup_info.h"
%include "enclave_info.h"
%include "contract.h"
%include "pdo_enclave.h"
%init %{
    InitializePDOEnclaveModule();
%}
