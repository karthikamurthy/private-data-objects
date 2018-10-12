#!/usr/bin/env python

# Copyright 2018 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import sys
import time
import argparse
import random
import json
import pdo.test.helpers.secrets as secret_helper

import pdo.eservice.pdo_helper as enclave_helper
import pdo.service_client.enclave as eservice_helper
import pdo.service_client.provisioning as pservice_helper

import pdo.contract as contract_helper
import pdo.common.crypto as crypto
import pdo.common.keys as keys
import pdo.common.secrets as secrets

import logging
logger = logging.getLogger(__name__)

# this will be used to test transaction dependencies
txn_dependencies = []

# representation of the enclave
enclave = None

# -----------------------------------------------------------------
def CreateAndRegisterEnclave(config) :
    try :
        enclave = enclave_helper.Enclave.create_new_enclave()
    except Exception as e :
        logger.error('failed to initialize the enclave; %s', str(e))
        sys.exit(-1)

    # save the data to a file
    data_dir = config['EnclaveData']['DataPath']
    basename = config['EnclaveData']['BaseName']

    logger.info("data_dir %s", data_dir)
    logger.info("basename %s", basename)
    enclave.save_to_file(basename, data_dir)

    pem_file_name = os.path.realpath(os.path.join(data_dir, "rsa_pub_key" + ".pem"))
    try:
        logger.info('save result data to %s', pem_file_name)
        with open(pem_file_name, "w") as file:
            file.write(enclave.encryption_key)
    except:
        logger.error("ERROR: Failed to write to file %s", pem_file_name)

    return enclave

# -----------------------------------------------------------------
def LoadEnclaveData(config) :
    data_dir = config['EnclaveData']['DataPath']
    basename = config['EnclaveData']['BaseName']

    logger.info("data_dir %s", data_dir)
    logger.info("basename %s", basename)

    try :
        enclave = enclave_helper.Enclave.read_from_file(basename, data_dir = data_dir)
    except FileNotFoundError as fe :
        logger.error("enclave information file missing; {0}".format(fe.filename))
        return None
    except Exception as e :
        logger.error("problem loading enclave information; %s", str(e))
        raise e

    return enclave

# -----------------------------------------------------------------
def ExecuteWorkOrder(config, enclave, indent=4) :
    try :
        wo_request=contract_helper.request.ContractRequest(
            'initialize',
            "",
            enclave,
            None,
            work_order=input_json_str, expression='1,1')

        wo_response = wo_request.evaluate()

        try:
            json_str = json.dumps(wo_response.result, indent=indent)
            logger.info('Response:\n%s', json_str)

            try:
                logger.info('save result data to %s', output_json_file_name)
                with open(output_json_file_name, "w") as file:
                    file.write(json_str)
            except:
                logger.error("ERROR: Failed to write to file %s", output_json_file_name)
        except:
            logger.error("ERROR: Failed to serialize JSON")


    except Exception as e :
        logger.error('failed to execute work order; %s', str(e))
        sys.exit(-1)

# -----------------------------------------------------------------
# -----------------------------------------------------------------
def LocalMain(config) :
    if not input_json_str:
        logger.error("JSON input file is not provided")
        exit(1)

    if not output_json_file_name:
        logger.error("JSON output file is not provided")
        exit(1)

    try:
        logger.debug('initialize the enclave')
        enclave_helper.initialize_enclave(config.get('EnclaveModule'))
    except:
        logger.exception('failed to initialize enclave; %s')
        sys.exit(-1)

    logger.info('loading the enclave')
    enclave = LoadEnclaveData(config)
    if not enclave:
        logger.info('creating a new enclave')
        enclave = CreateAndRegisterEnclave(config)

    logger.info('execute work order')
    ExecuteWorkOrder(config, enclave)

    exit(0)


## XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
## XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
## DO NOT MODIFY BELOW THIS LINE
## XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
## XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

## -----------------------------------------------------------------

ContractHost = os.environ.get("HOSTNAME", "localhost")
ContractHome = os.environ.get("CONTRACTHOME") or os.path.realpath("/opt/pdo")
ContractEtc = os.environ.get("CONTRACTETC") or os.path.join(ContractHome, "etc")
ContractKeys = os.environ.get("CONTRACTKEYS") or os.path.join(ContractHome, "keys")
ContractLogs = os.environ.get("CONTRACTLOGS") or os.path.join(ContractHome, "logs")
ContractData = os.environ.get("CONTRACTDATA") or os.path.join(ContractHome, "data")
LedgerURL = os.environ.get("LEDGER_URL", "http://127.0.0.1:8008/")
ScriptBase = os.path.splitext(os.path.basename(sys.argv[0]))[0]

config_map = {
    'base' : ScriptBase,
    'data' : ContractData,
    'etc'  : ContractEtc,
    'home' : ContractHome,
    'host' : ContractHost,
    'keys' : ContractKeys,
    'logs' : ContractLogs,
    'ledger' : LedgerURL
}


# -----------------------------------------------------------------
# -----------------------------------------------------------------
def ParseCommandLine(config, args) :
    global input_json_str
    global output_json_file_name

    parser = argparse.ArgumentParser()

    parser.add_argument('--logfile', help='Name of the log file, __screen__ for standard output', type=str)
    parser.add_argument('--loglevel', help='Logging level', type=str)
    parser.add_argument('--input_file', help='JSON input file name', type=str, default=None) # ??? [])
    parser.add_argument('--output_file', help='JSON output file name', type=str, default=None)  # ??? [])

    options = parser.parse_args(args)

    if config.get('Logging') is None :
        config['Logging'] = {
            'LogFile' : '__screen__',
            'LogLevel' : 'INFO'
        }
    if options.logfile :
        config['Logging']['LogFile'] = options.logfile
    if options.loglevel :
        config['Logging']['LogLevel'] = options.loglevel.upper()

    if options.input_file:
        try:
            logger.info('load JSON input from %s', options.input_file)
            with open(options.input_file, "r") as file:
                input_json_str = file.read()
        except:
            logger.error("ERROR: Failed to read from file %s", options.input_file)
            exit(1)
    else:
        input_json_str = None

    if options.output_file:
        output_json_file_name = options.output_file
    else:
        output_json_file_name = None


# -----------------------------------------------------------------
# -----------------------------------------------------------------
def Main() :
    import pdo.common.config as pconfig
    import pdo.common.logger as plogger

    # parse out the configuration file first
    conffiles = [ 'work_order_tests.toml' ]
    confpaths = [ ".", "./etc"]

    parser = argparse.ArgumentParser()
    parser.add_argument('--config', help='configuration file', nargs = '+')
    parser.add_argument('--config-dir', help='configuration folder', nargs = '+')
    (options, remainder) = parser.parse_known_args()

    if options.config :
        conffiles = options.config

    if options.config_dir :
        confpaths = options.config_dir

    global config_map
    config_map['identity'] = 'test-work-order'

    try :
        config = pconfig.parse_configuration_files(conffiles, confpaths, config_map)
        config_json_str = json.dumps(config, indent=4)
    except pconfig.ConfigurationException as e :
        logger.error(str(e))
        sys.exit(-1)

    ParseCommandLine(config, remainder)

    plogger.setup_loggers(config.get('Logging', {}))
    sys.stdout = plogger.stream_to_logger(logging.getLogger('STDOUT'), logging.DEBUG)
    sys.stderr = plogger.stream_to_logger(logging.getLogger('STDERR'), logging.WARN)

    LocalMain(config)

Main()
