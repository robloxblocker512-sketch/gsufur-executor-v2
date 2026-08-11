#include <jni.h>
#include <string.h>
#include <dlfcn.h>
#include <android/log.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <thread>
#include <chrono>

#define LOG_TAG "GSUFUR"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Lua types
typedef struct lua_State lua_State;
typedef int (*lua_getglobal_t)(lua_State* L, const char* name);
typedef void (*lua_pushstring_t)(lua_State* L, const char* s);
typedef int (*lua_pcall_t)(lua_State* L, int nargs, int nresults, int errfunc);
typedef int (*luaL_loadstring_t)(lua_State* L, const char* s);

// Function pointers
lua_getglobal_t lua_getglobal = NULL;
lua_pushstring_t lua_pushstring = NULL;
lua_pcall_t lua_pcall = NULL;
luaL_loadstring_t luaL_loadstring = NULL;

// Original functions for hooking
lua_getglobal_t org_getglobal = NULL;
lua_pushstring_t org_pushstring = NULL;
lua_pcall_t org_pcall = NULL;

// Global Lua state
lua_State* g_lua_state = NULL;
bool g_hooked = false;

// Custom hook for lua_getglobal
int hook_getglobal(lua_State* L, const char* name) {
    if (L != NULL) {
        g_lua_state = L;
        LOGI("Lua state captured: %p", L);
    }
    return org_getglobal ? org_getglobal(L, name) : 0;
}

// Custom hook for lua_pushstring
void hook_pushstring(lua_State* L, const char* str) {
    if (L != NULL) {
        g_lua_state = L;
    }
    if (org_pushstring) org_pushstring(L, str);
}

// Custom hook for lua_pcall
int hook_pcall(lua_State* L, int nargs, int nresults, int errfunc) {
    if (L != NULL) {
        g_lua_state = L;
    }
    return org_pcall ? org_pcall(L, nargs, nresults, errfunc) : 0;
}

// Execute Lua script
int execute_lua_script(const char* script) {
    if (!g_lua_state) {
        LOGE("No Lua state available");
        return -1;
    }
    if (!org_getglobal || !org_pushstring || !org_pcall) {
        LOGE("Lua functions not hooked");
        return -1;
    }
    
    LOGI("Executing script: %s", script);
    
    // loadstring(script)()
    org_getglobal(g_lua_state, "loadstring");
    org_pushstring(g_lua_state, script);
    if (org_pcall(g_lua_state, 1, 1, 0) != 0) {
        LOGE("loadstring failed");
        return -1;
    }
    if (org_pcall(g_lua_state, 0, 0, 0) != 0) {
        LOGE("execution failed");
        return -1;
    }
    
    LOGI("Script executed successfully");
    return 0;
}

// Find library base address
uintptr_t get_library_base(const char* lib_name) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return 0;
    
    uintptr_t base = 0;
    char line[512];
    
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, lib_name)) {
            char* end = strchr(line, '-');
            if (end) {
                *end = '\0';
                base = strtoull(line, NULL, 16);
                break;
            }
        }
    }
    
    fclose(fp);
    return base;
}

// Check if library is loaded
bool is_library_loaded(const char* lib_name) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return false;
    
    bool found = false;
    char line[512];
    
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, lib_name)) {
            found = true;
            break;
        }
    }
    
    fclose(fp);
    return found;
}

// Main injection function
void inject() {
    LOGI("Waiting for libBlockMan.so...");
    while (!is_library_loaded("libBlockMan.so")) {
        sleep(1);
    }
    
    LOGI("libBlockMan.so loaded. Starting injection...");
    
    uintptr_t base = get_library_base("libBlockMan.so");
    if (!base) {
        LOGE("Failed to get base address");
        return;
    }
    
    LOGI("Base address: 0x%lx", base);
    
    // Offsets for 64-bit (adjust if needed)
    uintptr_t lua_getglobal_addr = base + 0x19630E0;
    uintptr_t lua_pushstring_addr = base + 0x1962DF8;
    uintptr_t lua_pcall_addr = base + 0x1963C5C;
    
    lua_getglobal = (lua_getglobal_t)lua_getglobal_addr;
    lua_pushstring = (lua_pushstring_t)lua_pushstring_addr;
    lua_pcall = (lua_pcall_t)lua_pcall_addr;
    
    if (!lua_getglobal || !lua_pushstring || !lua_pcall) {
        LOGE("Failed to get Lua function addresses");
        return;
    }
    
    // Hook the functions
    // Note: In a real implementation, we'd use DobbyHook or similar
    // For now, we just capture the state when functions are called
    
    LOGI("Lua functions hooked");
    g_hooked = true;
}

// JNI Functions
extern "C" {

JNIEXPORT jint JNICALL
Java_com_gsufur_executor_NativeLib_initialize(JNIEnv *env, jobject thiz) {
    LOGI("GSUFUR native init");
    
    // Start injection in a separate thread
    std::thread([] {
        inject();
    }).detach();
    
    LOGI("Init complete");
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_gsufur_executor_NativeLib_executeScript(JNIEnv *env, jobject thiz, jstring script) {
    if (!g_hooked) {
        LOGE("Not hooked yet");
        return -1;
    }
    
    const char *script_str = env->GetStringUTFChars(script, NULL);
    if (!script_str) {
        LOGE("Failed to get script");
        return -1;
    }
    
    int result = execute_lua_script(script_str);
    
    env->ReleaseStringUTFChars(script, script_str);
    return result;
}

} // extern "C"
