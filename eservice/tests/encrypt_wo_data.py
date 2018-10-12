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

import pdo.common.crypto as crypto
import logging
import sys
import base64
import json
import os
import argparse

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)
logger = logging.getLogger()
logging.basicConfig(level=logging.DEBUG)


class WoException(Exception):
    pass


def check_file_exists(fname):
    try:
        with open(fname) as fd:
            fd.close()
            return True
    except:
        return False


def load_file(fname, def_content=None):
    try:
        with open(fname) as fd:
            try:
                content = fd.read().strip()
                fd.close()
                return content
            except OSError as err:
                print('Failed to read file {0}: {1}'.format(fname, err))
                raise WoException("ERROR: Failed to read file: " + fname)
    except TypeError as err:
        if def_content != None:
            return def_content
        print('Failed to open file {0}: {1}'.format(fname, err))
        raise WoException("ERROR: Failed to open file: " + fname)
    except OSError as err:
        if def_content != None:
            return def_content
        print('Failed to open file {0}: {1}'.format(fname, err))
        raise WoException("ERROR: Failed to open file: " + fname)


def save_file(fname, content):
    try:
        with open(fname, "w") as fdw:
            try:
                fdw.write(content)
                fdw.close()
            except OSError as err:
                print('Failed to write file {0}: {1}'.format(fname, err))
                raise WoException("ERROR: Failed to write file: " + fname)
    except OSError as err:
        print('Failed to open file for writing {0}: {1}'.format(fname, err))
        raise WoException("ERROR: Failed to open file for writing: " + fname)


def load_json(fname, def_content=None):
    json_str=load_file(fname, def_content)
    try:
        return json.loads(json_str)
    except:
        raise WoException("ERROR: Failed to parse JSON file: " + fname)


def save_json(fname, json_obj, indent=4):
    try:
        json_str=json.dumps(json_obj, indent=indent)
    except:
        raise WoException("ERROR: Failed to serialize JSON file: " + fname)
    save_file(fname, json_str)


def get_aes_key(aes_keys_json, data_type, no_new_keys=False):
    key = aes_keys_json.get(data_type)
    if key:
        return key
    elif no_new_keys:
        raise WoException("ERROR: No symmetric encryption key found for Type: " + data_type)

    try:
        key = crypto.SKENC_GenerateKey()
        key = base64.b64encode(bytes(key)).decode()
        aes_keys_json[data_type]=key
        return key
    except Exception as exc:
        raise WoException("ERROR: Symmetric encryption key generation failed: " + str(exc))


def get_json_value(json_dict, name, default_value=None):
    try:
        return json_dict[name]
    except:
        if default_value != None:
            return default_value
        str="ERROR: Failed to retrieve JSON value {0}".format(name)
        raise WoException(str)


def encrypt_data_item(aes_keys_json, rsa_pem, data_item_json):
    data_type = get_json_value(data_item_json, "Type")
    aes_key_b64=get_aes_key(aes_keys_json, data_type)

    aes_key=base64.b64decode(aes_key_b64)
    blob_original=get_json_value(data_item_json, "BLOB", "")
    print("AES key for '{0}': {1}".format(data_type, aes_key_b64))

    if blob_original:
        blob_encrypted=crypto.SKENC_EncryptMessage(aes_key, blob_original.encode())
        blob_encrypted_b64=base64.b64encode(bytes(blob_encrypted)).decode()
        data_item_json["BLOB"]=blob_encrypted_b64

    rpk = crypto.PKENC_PublicKey(rsa_pem)
    aes_key_encrypted=rpk.EncryptMessage(aes_key)
    aes_key_encrypted_b64=base64.b64encode(bytes(aes_key_encrypted)).decode()
    data_item_json["EncryptedDataEncryptionKey"]=aes_key_encrypted_b64


def decrypt_data_item(aes_keys_json, rsa_pem, data_item_json):
    data_type = get_json_value(data_item_json, "Type")

    if rsa_pem:
        rsk = crypto.PKENC_PrivateKey(rsa_pem)
        aes_key_encrypted_b64 = get_json_value(data_item_json, "EncryptedDataEncryptionKey")
        aes_key_encrypted=base64.b64decode(aes_key_encrypted_b64.encode())
        aes_key=bytes(rsk.DecryptMessage(aes_key_encrypted))
        print("AES key from PEM for '{0}': {1}".format(data_type, base64.b64encode(aes_key).decode()))
    else:
        aes_key_b64=get_aes_key(aes_keys_json, data_type, True)
        aes_key=base64.b64decode(aes_key_b64)
        print("AES key from file for '{0}': {1}".format(data_type, aes_key_b64))

    blob_encrypted_str = get_json_value(data_item_json, "BLOB", "")
    if blob_encrypted_str:
        blob_encrypted=base64.b64decode(blob_encrypted_str)
        blob_decrypted=crypto.SKENC_DecryptMessage(aes_key, blob_encrypted)
        blob_decrypted_str=bytes(blob_decrypted).decode()
        data_item_json["BLOB"] = blob_decrypted_str

    data_item_json["EncryptedDataEncryptionKey"] = ""


def process_json(aes_keys_json, rsa_pem, json_dict, do_encrypt, obj_name=None):
    if obj_name:
        json_obj = get_json_value(json_dict, obj_name)
    else:
        json_obj = get_json_value(json_dict, "params", "")
        if not json_obj:
            try:
                json_obj = get_json_value(json_dict, "result")
            except Exception as e:
                raise WoException("ERROR: Failed to retrieve JSON object for both 'params' and 'result'")

    data_array = get_json_value(json_obj, "Data")
    for data_item_json in data_array:
        if do_encrypt:
            encrypt_data_item(aes_keys_json, rsa_pem, data_item_json)
        else:
            decrypt_data_item(aes_keys_json, rsa_pem, data_item_json)


# rsa_pem_fname - is a name of public or private RSA key PEM file depending on value of do_encrypt
# aes_keys_json_fname - JSON file name for storing AES keys.
#       in case of encryption, if the file doesn't exist, generate keys and create the file, otherwise use existing keys
#       in case of decryption, this file must exist if RSA private key PEM file is not provided (rsa_pem_fname)
# input_json_fname - input JSON file name
# output_json_fname - output JSON file name
# do_encrypt is a Boolean flag indicating what operation to perform - encryption (true) or decryption (false)
def process(rsa_pem_fname,
            aes_keys_json_fname,
            input_json_fname,
            output_json_fname,
            do_encrypt=True,
            obj_name=None):
    rsa_pem=load_file(rsa_pem_fname, None if do_encrypt else "")

    aes_keys_json_str=load_file(aes_keys_json_fname, "{}")
    aes_keys_json=json.loads(aes_keys_json_str)

    json_dict = load_json(input_json_fname)

    process_json(aes_keys_json, rsa_pem, json_dict, do_encrypt, obj_name)
    if do_encrypt:
        save_json(output_json_fname, json_dict, indent=4)
        save_json(aes_keys_json_fname, aes_keys_json, indent=4)
    else:
        save_json(output_json_fname, json_dict, indent=4)

    print("\nCompleted successfully\n")


# rsa_pem_fname - is a name of public or private RSA key PEM file depending on value of do_encrypt
# aes_keys_json_fname - JSON file name for storing AES keys.
#       in case of encryption, if the file doesn't exist, generate keys and create the file, otherwise use existing keys
#       in case of decryption, this file must exist if RSA private key PEM file is not provided (rsa_pem_fname)
# input_json_fname_base - is an input file name base to append suffixes .001, .002, ... until the file exists
# output_json_fname_base - is an output file name base to append suffixes .001, .001, ...
# do_encrypt is a Boolean flag indicating what operation to perform - encryption (true) or decryption (false)
def process_file_list(
        rsa_pem_fname,
        aes_keys_json_fname,
        input_json_fname_base,
        output_json_fname_base,
        do_encrypt=True,
        obj_name=None):
    index = 1
    input_json_fname = "{0}{1:03}".format(input_json_fname_base, index)

    print("\\nChecking file:", input_json_fname)

    while check_file_exists(input_json_fname):
        print("\\nProcessing file:", input_json_fname)
        output_json_fname = "{0}{1:03}".format(output_json_fname_base, index)
        process(rsa_pem_fname, aes_keys_json_fname, input_json_fname, output_json_fname, do_encrypt, obj_name)

        index = index + 1
        input_json_fname = "{0}{1:03}".format(input_json_fname_base, index)
        print("\\nChecking file:", input_json_fname)

    if index == 1:
        print("\nNothing to process\n")


def execute_def_encrypt():
    process(
        rsa_pem_fname="rsa_pub.pem",
        aes_keys_json_fname="aes_keys.json",
        decrypted_json_fname="unencrypted_test_input.json",
        encrypted_json_fname="encrypted_test_input.json")


def execute_def_decrypt():
    process(
        rsa_pem_fname= "rsa_prv.pem",
        aes_keys_json_fname="aes_keys.json",
        decrypted_json_fname="deccrypted_test_input.json",
        encrypted_json_fname="encrypted_test_input.json",
        do_encrypt=False)


def create_parent_parser(prog_name):
    parent_parser = argparse.ArgumentParser(prog=prog_name, add_help=False)

    parent_parser.add_argument(
        '-V', '--version',
        action='version',
        version=('Verifiable Compute JSON input/output data encryption and decryption - 0.1'),
        help='display version information')

    return parent_parser


def create_parser(prog_name):
    parent_parser = create_parent_parser(prog_name)

    parser = argparse.ArgumentParser(
        parents=[parent_parser],
        formatter_class=argparse.RawDescriptionHelpFormatter)

    subparsers = parser.add_subparsers(title='subcommands', dest='command')

    add_encryption_parser(subparsers, parent_parser)
    add_decryption_parser(subparsers, parent_parser)

    return parser


def add_encryption_parser(subparsers, parent_parser):
    message = 'Encrypt BLOBs in "Data" arrays items'

    parser = subparsers.add_parser(
        'encrypt',
        parents=[parent_parser],
        description=message,
        help='Encrypt BLOBs in "Data" arrays items within a JSON file')

    parser.add_argument(
        '-a', '--aes-key-file',
        type=str,
        help="Name of a JSON file with AES keys, it is auto generated if doesn't exist",
        default=None)

    parser.add_argument(
        '-r', '--rsa_pub_pem',
        type=str,
        help="RSA public key PEM file name",
        default=None)

    parser.add_argument(
        '-i', '--input_json',
        type=str,
        help="File name of the JSON input with clear text data",
        default=None)

    parser.add_argument(
        '-o', '--output_json',
        type=str,
        help="File name of the JSON output for encrypted data",
        default=None)

    parser.add_argument(
        '-p', '--param_obj_name',
        type=str,
        help='Name of a JSON object with "Data" array; default is "params"',
        default="params")

    parser.add_argument(
        '-l', '--file_list',
        action='store_true',
        help='If set, processing multiple files by adding num suffixes from 001 up to 999 to input and output JSON file names',
        default=False)


def add_decryption_parser(subparsers, parent_parser):
    message = 'Decrypt BLOBs in "Data" arrays items'

    parser = subparsers.add_parser(
        'decrypt',
        parents=[parent_parser],
        description=message,
        help='Decrypt BLOBs in "Data" arrays items within a JSON file')

    parser.add_argument(
        '-a', '--aes-key-file',
        type=str,
        help="Name of JSON file with AES keys. Optional, if RSA private key is provided",
        default=None)

    parser.add_argument(
        '-r', '--rsa_prv_pem',
        type=str,
        help="RSA private key PEM file name. Optional, if AES keys file provided",
        default=None)

    parser.add_argument(
        '-i', '--input_json',
        type=str,
        help="File name of the JSON input with encrypted data",
        default=None)

    parser.add_argument(
        '-o', '--output_json',
        type=str,
        help="File name of the JSON output for decrypted data",
        default=None)

    parser.add_argument(
        '-p', '--param_obj_name',
        type=str,
        help='Name of a JSON object with "Data" array, optional. default is "result"',
        default="result")

    parser.add_argument(
        '-l', '--file_list',
        action='store_true',
        help='If set, processing multiple files by adding num suffixes from 001 up to 999 to input and output JSON file names',
        default=False)


def check_param(name, value, required, on_error):
    if value:
        print("{0}: {1}".format(name, value))
    elif required:
        print("\n\nERROR: Parameter '{0}' is required\n{1}".format(name, on_error))
        sys.exit(1)


def Main(prog_name=os.path.basename(sys.argv[0]), args=None):
    if args is None:
        args = sys.argv[1:]
    parser = create_parser(prog_name)
    args = parser.parse_args(args)

    if not args.command:
        parser.print_help()
        sys.exit(1)

    try:
        if args.command == 'encrypt':
            print("\nEncrypting...")
            check_param(
                "RSA public key PEM file name",
                args.rsa_pub_pem,
                True,
                "Use command line option 'rsa_pub_key'\nFor help type 'encrypt_wo_data.py encrypt help'")
            check_param(
                "AES key file name",
                args.aes_key_file,
                True,
                "Use command line option 'aes_key_file'\nFor help type 'encrypt_wo_data.py encrypt help'")
            check_param(
                "Input JSON file name",
                args.input_json,
                True,
                "Use command line option 'input_json'\nFor help type 'encrypt_wo_data.py encrypt help'")
            check_param(
                "Output JSON file name",
                args.output_json,
                True,
                "Use command line option 'input_json'\nFor help type 'encrypt_wo_data.py encrypt help'")
            check_param(
                "JSON parameter object name",
                args.param_obj_name,
                True,
                "Use command line option 'param_obj_name'\nFor help type 'encrypt_wo_data.py encrypt help'")
            print()
            if args.file_list:
                process_file_list(
                    args.rsa_pub_pem,
                    args.aes_key_file,
                    args.input_json,
                    args.output_json,
                    True,
                    args.param_obj_name)
            else:
                process(
                args.rsa_pub_pem,
                args.aes_key_file,
                args.input_json,
                args.output_json,
                True,
                args.param_obj_name)
        elif args.command == 'decrypt':
            print("\nDecrypting...")
            check_param(
                "RSA private key PEM file name",
                args.rsa_prv_pem,
                False,
                "Use command line option 'rsa_pub_key'\nFor help type 'encrypt_wo_data.py decrypt help'")
            check_param(
                "AES key file name",
                args.aes_key_file,
                False,
                "Use command line option 'aes_key_file'\nFor help type 'encrypt_wo_data.py decrypt help'")
            check_param(
                "Input JSON file name",
                args.input_json,
                True,
                "Use command line option 'input_json'\nFor help type 'encrypt_wo_data.py decrypt help'")
            check_param(
                "Output JSON file name",
                args.output_json,
                True,
                "Use command line option 'input_json'\nFor help type 'encrypt_wo_data.py decrypt help'")
            check_param(
                "JSON parameter object name",
                args.param_obj_name,
                True,
                "Use command line option 'param_obj_name'\nFor help type 'encrypt_wo_data.py decrypt help'")
            print()
            if args.file_list:
                process_file_list(
                    args.rsa_prv_pem,
                    args.aes_key_file,
                    args.input_json,
                    args.output_json,
                    False,
                    args.param_obj_name)
            else:
                process(
                    args.rsa_prv_pem,
                    args.aes_key_file,
                    args.input_json,
                    args.output_json,
                    False,
                    args.param_obj_name)
        else:
            parser.print_help()
            raise WoException("Invalid command: {}".format(args.command))
    except Exception as e:
        print('{0}'.format(str(e)))


Main()