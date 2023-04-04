#include <node.h>
#include <windows.h>

namespace ctrlc {

  using v8::FunctionCallbackInfo;
  using v8::Isolate;
  using v8::Local;
  using v8::Object;
  using v8::String;
  using v8::Value;

  void StopProgram(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    
    // Check the number of arguments passed
    if (args.Length() != 1) {
      v8::Local<v8::String> v8String = v8::String::NewFromUtf8(isolate, "Invalid arguments").ToLocalChecked();
      isolate->ThrowException(v8::Exception::TypeError(v8String));

      return;
    }

    // Check the argument types
    if (!args[0]->IsUint32()) {
      v8::Local<v8::String> v8String = v8::String::NewFromUtf8(isolate, "Argument must be a number").ToLocalChecked();
      isolate->ThrowException(v8::Exception::TypeError(v8String));

      return;
    }


    DWORD dwProcessId = args[0]->Uint32Value(isolate->GetCurrentContext()).ToChecked();

    BOOL bSuccess = FALSE;
    HANDLE hProcess = NULL;

    // Open the process with PROCESS_CREATE_THREAD, PROCESS_QUERY_INFORMATION, and PROCESS_VM_OPERATION access rights.
    hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION, FALSE, dwProcessId);
    if (hProcess == NULL)
    {
      v8::Local<v8::String> v8String = v8::String::NewFromUtf8(isolate, "Failed to open process").ToLocalChecked();
      isolate->ThrowException(v8::Exception::Error(v8String));

      return;
    }

    // Generate a CTRL+C event in the target process.
    if (!GenerateConsoleCtrlEvent(CTRL_C_EVENT, dwProcessId))
    {
      v8::Local<v8::String> v8String = v8::String::NewFromUtf8(isolate, "Failed to generate CTRL+C event").ToLocalChecked();
      isolate->ThrowException(v8::Exception::Error(v8String));

      goto cleanup;
    }

    bSuccess = TRUE;

  // cleanup function to 
  cleanup:
    if (hProcess != NULL)
    {
      CloseHandle(hProcess);
    }

    // Return the result
    args.GetReturnValue().Set(v8::Boolean::New(isolate, bSuccess));
  }
  
  void Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "stopProgram", StopProgram);
  }

  NODE_MODULE(ctrlc, Init)

}  // namespace ctrlc
