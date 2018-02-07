/* Copyright 2018 Streampunk Media Ltd.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <node_api.h>
#include <assert.h>
#include <stdio.h>

#if defined _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

typedef struct {
  int32_t wait;
  napi_ref result;
  napi_deferred _deferred;
  napi_async_work _request;
} carrier;

carrier the_carrier;

void Execute(napi_env env, void* data) {
  printf("Starting to wait!\n");
  carrier* c = static_cast<carrier*>(data);
  #if defined _WIN32
    Sleep(c->wait);
  #else
    sleep(c->wait / 1000);
  #endif
  printf("Glad that is all over.\n");
  return;
}

void Complete(napi_env env, napi_status status, void* data) {
  printf("Complete called %i.\n", status);
  carrier* c = static_cast<carrier*>(data);
  napi_value resObj;
  napi_value resStr;

  status = napi_get_reference_value(env, c->result, &resObj);
  printf("Made undefined %i.\n", status);
  assert(status == napi_ok);

  status = napi_coerce_to_string(env, resObj, &resStr);
  assert(status == napi_ok);

  status = napi_resolve_deferred(env, c->_deferred, resStr);
  printf("Called resolve deferred %i.\n", status);
  assert(status == napi_ok);
  c->_deferred = NULL;

  status = napi_delete_async_work(env, c->_request);
  assert(status == napi_ok);
  status = napi_delete_reference(env, c->result);
  assert(status == napi_ok);
}

napi_value Method(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value promise;
  napi_value resource_name;

  size_t argc = 2;
  napi_value args[2];
  status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  assert(status == napi_ok);

  if (argc < 2) {
    napi_throw_type_error(env, nullptr, "Wrong number of arguments.");
    return nullptr;
  }

  napi_valuetype t;
  status = napi_typeof(env, args[0], &t);
  assert(status == napi_ok);
  if (t != napi_number) {
    napi_throw_type_error(env, nullptr, "First arugment is a number - the timeout time.");
    return nullptr;
  }

  status = napi_typeof(env, args[1], &t);
  assert(status == napi_ok);
  if (t != napi_string) {
    napi_throw_type_error(env, nullptr, "Second arugment is a string - the result.");
    return nullptr;
  }

  status = napi_get_value_int32(env, args[0], &the_carrier.wait);
  assert(status == napi_ok);

  napi_value resObj;
  status = napi_coerce_to_object(env, args[1], &resObj);
  assert(status == napi_ok);
  status = napi_create_reference(env, resObj, 1, &the_carrier.result);
  printf("Got reference create status %i\n", status);
  assert(status == napi_ok);

  status = napi_create_promise(env, &the_carrier._deferred, &promise);
  assert(status == napi_ok);

  status = napi_create_string_utf8(env, "WaitABit", NAPI_AUTO_LENGTH, &resource_name);
  assert(status == napi_ok);
  status = napi_create_async_work(env, NULL, resource_name, Execute,
    Complete, &the_carrier, &the_carrier._request);
  assert(status == napi_ok);
  status = napi_queue_async_work(env, the_carrier._request);
  assert(status == napi_ok);

  return promise;
}

#define DECLARE_NAPI_METHOD(name, func) { name, 0, func, 0, 0, 0, napi_default, 0 }

napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_property_descriptor desc = DECLARE_NAPI_METHOD("naprom", Method);
  status = napi_define_properties(env, exports, 1, &desc);
  assert(status == napi_ok);
  return exports;
}

NAPI_MODULE(hello, Init)
