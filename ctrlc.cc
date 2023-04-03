#include <node.h>
#include <windows.h>

namespace ctrlc {

  using v8::FunctionCallbackInfo;
  using v8::Isolate;
  using v8::Local;
  using v8::Object;
  using v8::String;
  using v8::Value;

  void StopProgram(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    // Check the number of arguments passed
    if (args.Length() < 1) {
      isolate->ThrowException(
          Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
      return;
    }

    // Check the argument types
    if (!args[0]->IsNumber()) {
      isolate->ThrowException(
          Exception::TypeError(String::NewFromUtf8(isolate, "Argument must be a number")));
      return;
    }

    DWORD processId = args[0]->Uint32Value();

    // Get handle to the process
    HANDLE hProcess = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, processId);
    if (hProcess == NULL) {
      isolate->ThrowException(
          Exception::Error(String::NewFromUtf8(isolate, "Failed to open process")));
      return;
    }

    // Attach to the console of the process
    if (!AttachConsole(processId)) {
      CloseHandle(hProcess);
      isolate->ThrowException(
          Exception::Error(String::NewFromUtf8(isolate, "Failed to attach to console")));
      return;
    }

    // Disable Ctrl-C handling for our program
    SetConsoleCtrlHandler(NULL, TRUE);

    // Send the Ctrl-C signal to the process
    BOOL success = GenerateConsoleCtrlEvent(CTRL_C_EVENT, processId);

    // Wait for the process to terminate
    DWORD waitResult = WaitForSingleObject(hProcess, 2000);

    // Re-enable Ctrl-C handling
    SetConsoleCtrlHandler(NULL, FALSE);

    // Detach from the console of the process
    FreeConsole();

    // Close the handle to the process
    CloseHandle(hProcess);

    // Return the result
    Local<Value> result = success && (waitResult == WAIT_OBJECT_0) ? True(isolate) : False(isolate);
    args.GetReturnValue().Set(result);
  }

  void Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "stopProgram", StopProgram);
  }

  NODE_MODULE(ctrlc, Init)

}  // namespace ctrlc
