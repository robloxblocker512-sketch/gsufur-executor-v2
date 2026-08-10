#include <jni.h>
#include <string.h>
#include <dlfcn.h>
#include <android/log.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdbool.h>

#define LOG_TAG "GSUFUR"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Lua types and function pointers
typedef struct lua_State lua_State;
typedef int (*luaL_dostring_t)(lua_State* L, const char* str);
typedef void (*lua_pushstring_t)(lua_State* L, const char* s);
typedef int (*lua_pcall_t)(lua_State* L, int nargs, int nresults, int errfunc);

// Function pointers
luaL_dostring_t luaL_dostring = NULL;
lua_pushstring_t lua_pushstring = NULL;
lua_pcall_t lua_pcall = NULL;

// Global Lua state
lua_State* g_lua_state = NULL;
bool g_hook_installed = false;

// Hook for lua_pushstring to capture the state
lua_State* (*original_lua_newstate)(void* f, void* ud) = NULL;
void (*original_lua_close)(lua_State* L) = NULL;

lua_State* hooked_lua_newstate(void* f, void* ud) {
    lua_State* L = original_lua_newstate(f, ud);
    if (L && !g_lua_state) {
        g_lua_state = L;
        LOGI("Lua state captured at: %p", L);
    }
    return L;
}

void hooked_lua_close(lua_State* L) {
    if (L == g_lua_state) {
        g_lua_state = NULL;
        LOGI("Lua state closed");
    }
    original_lua_close(L);
}

// Execute a Lua script
int execute_lua_script(lua_State* L, const char* script) {
    if (!L) {
        LOGE("No Lua state available");
        return -1;
    }
    if (!luaL_dostring) {
        LOGE("luaL_dostring not available");
        return -1;
    }
    LOGI("Executing script: %s", script);
    return luaL_dostring(L, script);
}

// Find Lua library and hook functions
int find_lua_library() {
    // Try different possible names
    const char* lib_names[] = {
        "liblua.so",
        "liblua5.1.so",
        "liblua5.2.so",
        "liblua5.3.so",
        "liblua5.4.so",
        "liblua51.so",
        "liblua52.so",
        "liblua53.so",
        "libluajit.so",
        NULL
    };
    
    void* lua_handle = NULL;
    
    for (int i = 0; lib_names[i] != NULL; i++) {
        lua_handle = dlopen(lib_names[i], RTLD_LAZY);
        if (lua_handle) {
            LOGI("Found Lua library: %s", lib_names[i]);
            break;
        }
    }
    
    if (!lua_handle) {
        LOGE("Failed to find Lua library");
        return -1;
    }
    
    // Get function pointers
    luaL_dostring = (luaL_dostring_t)dlsym(lua_handle, "luaL_dostring");
    if (!luaL_dostring) {
        // Try alternative names
        luaL_dostring = (luaL_dostring_t)dlsym(lua_handle, "luaL_dostring");
        if (!luaL_dostring) {
            LOGE("Failed to find luaL_dostring");
            return -1;
        }
    }
    
    lua_pushstring = (lua_pushstring_t)dlsym(lua_handle, "lua_pushstring");
    lua_pcall = (lua_pcall_t)dlsym(lua_handle, "lua_pcall");
    
    // Hook lua_newstate to capture the Lua state
    original_lua_newstate = (lua_State* (*)(void*, void*))dlsym(lua_handle, "lua_newstate");
    if (original_lua_newstate) {
        LOGI("Found lua_newstate, hooking...");
        // In a real implementation, we would use mprotect to write to the function address
        // For now, we'll scan for the state instead
    }
    
    LOGI("Lua functions loaded successfully");
    return 0;
}

// Scan memory for the Lua state
lua_State* scan_for_lua_state() {
    // We can scan for known Lua values in memory
    // This is a simplified version - real implementation would be more sophisticated
    LOGI("Scanning for Lua state...");
    return NULL; // Placeholder
}

JNIEXPORT jint JNICALL
Java_com_gsufur_executor_NativeLib_initialize(JNIEnv *env, jobject thiz) {
    LOGI("GSUFUR native init");
    
    int result = find_lua_library();
    if (result != 0) {
        LOGI("Failed to initialize Lua library");
        return -1;
    }
    
    g_hook_installed = true;
    LOGI("Initialization complete");
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_gsufur_executor_NativeLib_executeScript(JNIEnv *env, jobject thiz, jstring script) {
    if (!g_hook_installed) {
        LOGE("Hook not installed");
        return -1;
    }
    
    const char *script_str = (*env)->GetStringUTFChars(env, script, NULL);
    if (!script_str) {
        LOGE("Failed to get script string");
        return -1;
    }
    
    int result = -1;
    
    // Try to get Lua state from scan
    if (!g_lua_state) {
        LOGE("No Lua state available");
        result = -1;
    } else {
        result = execute_lua_script(g_lua_state, script_str);
    }
    
    (*env)->ReleaseStringUTFChars(env, script, script_str);
    
    if (result == 0) {
        LOGI("Script executed successfully");
    } else {
        LOGE("Script execution failed with code: %d", result);
    }
    
    return result;
}
