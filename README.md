# NAPI<a name="EN-US_TOPIC_0000001149901711"></a>

-   [Introduction](#section11660541593)
-   [Directory Structure](#section161941989596)
-   [When to Use](#section11759141594811)
-   [Available APIs](#section1611515555510)
-   [How to Develop](#section937267212)
-   [Repositories Involved](#section3970193518214)

## Introduction<a name="section11660541593"></a>

The  **foundation/ace/napi**  repository contains a development framework for extending the JS Native Module and provides APIs developed based on Node.js N-API for external use.

**Figure  1**  Architecture<a name="fig1049423884819"></a>  


![](figures/en-us_image_0000001162437581.png)

-   **NativeEngine**

    NativeEngine is the JS engine abstraction layer. It unifies API behavior of the JS engines at the NAPI layer.

-   **ModuleManager**

    ModuleManager is used to load modules and cache module information.

-   **ScopeManager**

    ScopeManager manages the NativeValue lifecycle.

-   **ReferenceManager**

    ReferenceManager manages the NativeReference lifecycle.


## Directory Structure<a name="section161941989596"></a>

The source code directory structure of this repository is as follows:

```
foundation/ace/napi
   ├── interfaces
   │   └── kits
   │       └── napi           # NAPI header files
   ├── module_manager         # Module manager
   ├── native_engine          # NativeEngine abstraction layer
   │   └── impl
   │       └── quickjs        # QuickJS-based NativeEngine implementation
   ├── scope_manager          # Scope manager
   └── test                   # Test code
```

## When to Use<a name="section11759141594811"></a>

NAPI is suitable for processing I/O- and CPU-intensive tasks and system tasks. It encapsulates the capabilities and provides them to apps as JS APIs. NAPI can be used to implement mutual access between JS and C/C++ code. You can use NAPI to develop modules such as network communications, serial port access, multimedia decoding, and sensor data collection.

## Available APIs<a name="section1611515555510"></a>

For details about the API implementation, see the  **foundation/ace/napi**  repository.

**Table  1**  Available NAPIs

<a name="table10789351555"></a>
<table><thead align="left"><tr id="row1878635175515"><th class="cellrowborder" valign="top" width="19.439999999999998%" id="mcps1.2.3.1.1"><p id="p27816352556"><a name="p27816352556"></a><a name="p27816352556"></a>Category</p>
</th>
<th class="cellrowborder" valign="top" width="80.56%" id="mcps1.2.3.1.2"><p id="p078835105514"><a name="p078835105514"></a><a name="p078835105514"></a>Description</p>
</th>
</tr>
</thead>
<tbody><tr id="row17863535520"><td class="cellrowborder" valign="top" width="19.439999999999998%" headers="mcps1.2.3.1.1 "><p id="p678143512552"><a name="p678143512552"></a><a name="p678143512552"></a>Module registration</p>
</td>
<td class="cellrowborder" valign="top" width="80.56%" headers="mcps1.2.3.1.2 "><p id="p4781035185515"><a name="p4781035185515"></a><a name="p4781035185515"></a>Registers module information with a manager.</p>
</td>
</tr>
<tr id="row67951538205618"><td class="cellrowborder" valign="top" width="19.439999999999998%" headers="mcps1.2.3.1.1 "><p id="p127961438195616"><a name="p127961438195616"></a><a name="p127961438195616"></a>Exception &amp; Error handling</p>
</td>
<td class="cellrowborder" valign="top" width="80.56%" headers="mcps1.2.3.1.2 "><p id="p47961338195617"><a name="p47961338195617"></a><a name="p47961338195617"></a>Throws exceptions.</p>
</td>
</tr>
<tr id="row778417510579"><td class="cellrowborder" valign="top" width="19.439999999999998%" headers="mcps1.2.3.1.1 "><p id="p1878575185715"><a name="p1878575185715"></a><a name="p1878575185715"></a>Object lifecycle</p>
</td>
<td class="cellrowborder" valign="top" width="80.56%" headers="mcps1.2.3.1.2 "><p id="p1178565175711"><a name="p1178565175711"></a><a name="p1178565175711"></a>Manages NAPI object lifecycle within limited scopes.</p>
</td>
</tr>
<tr id="row83179619572"><td class="cellrowborder" valign="top" width="19.439999999999998%" headers="mcps1.2.3.1.1 "><p id="p1531786125717"><a name="p1531786125717"></a><a name="p1531786125717"></a>JS object</p>
</td>
<td class="cellrowborder" valign="top" width="80.56%" headers="mcps1.2.3.1.2 "><p id="p12317663572"><a name="p12317663572"></a><a name="p12317663572"></a>Creates standard object types.</p>
</td>
</tr>
<tr id="row63859616579"><td class="cellrowborder" valign="top" width="19.439999999999998%" headers="mcps1.2.3.1.1 "><p id="p438696165720"><a name="p438696165720"></a><a name="p438696165720"></a>C-to-NAPI</p>
</td>
<td class="cellrowborder" valign="top" width="80.56%" headers="mcps1.2.3.1.2 "><p id="p14386462570"><a name="p14386462570"></a><a name="p14386462570"></a>Converts data types from C to NAPI.</p>
</td>
</tr>
<tr id="row1145119612578"><td class="cellrowborder" valign="top" width="19.439999999999998%" headers="mcps1.2.3.1.1 "><p id="p13451116115716"><a name="p13451116115716"></a><a name="p13451116115716"></a>NAPI-to-C</p>
</td>
<td class="cellrowborder" valign="top" width="80.56%" headers="mcps1.2.3.1.2 "><p id="p12451464574"><a name="p12451464574"></a><a name="p12451464574"></a>Converts data types from NAPI to C.</p>
</td>
</tr>
<tr id="row1451436155720"><td class="cellrowborder" valign="top" width="19.439999999999998%" headers="mcps1.2.3.1.1 "><p id="p155147625713"><a name="p155147625713"></a><a name="p155147625713"></a>Global instance</p>
</td>
<td class="cellrowborder" valign="top" width="80.56%" headers="mcps1.2.3.1.2 "><p id="p551466195713"><a name="p551466195713"></a><a name="p551466195713"></a>Obtains global instances.</p>
</td>
</tr>
<tr id="row65815617579"><td class="cellrowborder" valign="top" width="19.439999999999998%" headers="mcps1.2.3.1.1 "><p id="p458119617577"><a name="p458119617577"></a><a name="p458119617577"></a>JS value</p>
</td>
<td class="cellrowborder" valign="top" width="80.56%" headers="mcps1.2.3.1.2 "><p id="p125818625711"><a name="p125818625711"></a><a name="p125818625711"></a>Provides APIs executing <strong id="b18893622126"><a name="b18893622126"></a><a name="b18893622126"></a>===</strong>, <strong id="b5190121451212"><a name="b5190121451212"></a><a name="b5190121451212"></a>typeof</strong>, <strong id="b01299162121"><a name="b01299162121"></a><a name="b01299162121"></a>instanceof</strong>, and other operations alike.</p>
</td>
</tr>
<tr id="row10649166145711"><td class="cellrowborder" valign="top" width="19.439999999999998%" headers="mcps1.2.3.1.1 "><p id="p18649196195716"><a name="p18649196195716"></a><a name="p18649196195716"></a>JS object property</p>
</td>
<td class="cellrowborder" valign="top" width="80.56%" headers="mcps1.2.3.1.2 "><p id="p164919615574"><a name="p164919615574"></a><a name="p164919615574"></a>Provides functions for performing operations on object properties.</p>
</td>
</tr>
<tr id="row0714260574"><td class="cellrowborder" valign="top" width="19.439999999999998%" headers="mcps1.2.3.1.1 "><p id="p37143675714"><a name="p37143675714"></a><a name="p37143675714"></a>JS function</p>
</td>
<td class="cellrowborder" valign="top" width="80.56%" headers="mcps1.2.3.1.2 "><p id="p771466115711"><a name="p771466115711"></a><a name="p771466115711"></a>Invokes functions and creates instances.</p>
</td>
</tr>
<tr id="row1578176155717"><td class="cellrowborder" valign="top" width="19.439999999999998%" headers="mcps1.2.3.1.1 "><p id="p1782126155711"><a name="p1782126155711"></a><a name="p1782126155711"></a>Object encapsulation</p>
</td>
<td class="cellrowborder" valign="top" width="80.56%" headers="mcps1.2.3.1.2 "><p id="p87821967572"><a name="p87821967572"></a><a name="p87821967572"></a>Binds the external context of JS objects.</p>
</td>
</tr>
<tr id="row8854116105717"><td class="cellrowborder" valign="top" width="19.439999999999998%" headers="mcps1.2.3.1.1 "><p id="p1685414619578"><a name="p1685414619578"></a><a name="p1685414619578"></a>Simple asynchronization</p>
</td>
<td class="cellrowborder" valign="top" width="80.56%" headers="mcps1.2.3.1.2 "><p id="p1685411612574"><a name="p1685411612574"></a><a name="p1685411612574"></a>Creates asynchronous tasks.</p>
</td>
</tr>
<tr id="row109154635718"><td class="cellrowborder" valign="top" width="19.439999999999998%" headers="mcps1.2.3.1.1 "><p id="p391556195720"><a name="p391556195720"></a><a name="p391556195720"></a>Promise</p>
</td>
<td class="cellrowborder" valign="top" width="80.56%" headers="mcps1.2.3.1.2 "><p id="p1091566135717"><a name="p1091566135717"></a><a name="p1091566135717"></a>Creates a function set for a promise.</p>
</td>
</tr>
<tr id="row1671493313016"><td class="cellrowborder" valign="top" width="19.439999999999998%" headers="mcps1.2.3.1.1 "><p id="p1971512331709"><a name="p1971512331709"></a><a name="p1971512331709"></a>Script</p>
</td>
<td class="cellrowborder" valign="top" width="80.56%" headers="mcps1.2.3.1.2 "><p id="p371517332013"><a name="p371517332013"></a><a name="p371517332013"></a>Runs JS code.</p>
</td>
</tr>
</tbody>
</table>

## How to Develop<a name="section937267212"></a>

The following example describes how to use NAPI to develop a JS API for obtaining the application bundle name.

The prototype of the JS API is as follows:

```
function getAppName(): string;
```

The implementation code of the JS API is as follows:

```
// app.cpp
#include <stdio.h>
#include <string.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"

struct AsyncCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
};

// C/C++ function corresponding to getAppName()
napi_value JSGetAppName(napi_env env, napi_callback_info info) {
    napi_deferred deferred;
    napi_value promise;
    // Create a promise.
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));

    AsyncCallbackInfo* asyncCallbackInfo = new AsyncCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
        .deferred = deferred,
    };

    napi_value resourceName;
    napi_create_string_latin1(env, "GetAppName", NAPI_AUTO_LENGTH, &resourceName);
    // Create a queue of asynchronous tasks.
    napi_create_async_work(
        env, nullptr, resourceName,
        // Callback for an asynchronous task
        [](napi_env env, void* data) {},
        // Callback after the asynchronous task is complete
        [](napi_env env, napi_status status, void* data) {
            AsyncCallbackInfo* asyncCallbackInfo = (AsyncCallbackInfo*)data;
            napi_value appName;
            const char* str = "com.example.helloworld";
            napi_create_string_utf8(env, str, strlen(str), &appName);
            // Trigger the callback.
            napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, appName);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void*)asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return promise;
}

// Export the module.
static napi_value AppExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getAppName", JSGetAppName),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

// App module description
static napi_module appModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = AppExport,
    .nm_modname = "app",
    .nm_priv = ((void*)0),
    .reserved = {0}
};

// Register the module.
extern "C" __attribute__((constructor)) void AppRegister()
{
    napi_module_register(&appModule);
}
```

The build script is as follows:

```
// BUILD.gn
import("//build/ohos.gni")
ohos_shared_library("app") {
  # Specify the source file to build.
  sources = [
    "app.cpp",
  ]
  # Specify build dependencies.
  deps = [ "//foundation/ace/napi:ace_napi" ]
  # Specify the directory where the library is generated.
  relative_install_dir = "module"
  subsystem_name = "ace"
  part_name = "napi"
}
```

Test code to run in your app is as follows:

```
import app from '@ohos.app'
export default {
  testGetAppName() {
    app.getAppName().then(function (data) {
      console.info('app name: ' + data);
    });
  }
}
```

## Repositories Involved<a name="section3970193518214"></a>

JS UI framework

ace\_ace\_engine

ace\_ace\_engine\_lite

**ace\_napi**

