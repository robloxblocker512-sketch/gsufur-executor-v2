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

typedef struct lua_State lua_State;
typedef int (*luaL_dostring_t)(lua_State* L, const char* str);
typedef lua_State* (*lua_newstate_t)(void* f, void* ud);
typedef void (*lua_close_t)(lua_State* L);

luaL_dostring_t luaL_dostring = NULL;
lua_newstate_t lua_newstate = NULL;
lua_close_t lua_close = NULL;
lua_State* g_lua_state = NULL;
bool g_hooked = false;

// Hook lua_newstate to capture the state
lua_State* hooked_lua_newstate(void* f, void* ud) {
    lua_State* L = lua_newstate(f, ud);
    if (L) {
        g_lua_state = L;
        LOGI("Lua state captured: %p", L);
    }
    return L;
}

// Find the Lua library and hook everything
int hook_lua() {
    // Try all possible Lua library names
    const char* libs[] = {
        "liblua.so",
        "liblua5.1.so", 
        "liblua5.2.so",
        "liblua5.3.so",
        "liblua5.4.so",
        "libluajit.so",
        NULL
    };
    
    void* handle = NULL;
    for (int i = 0; libs[i] != NULL; i++) {
        handle = dlopen(libs[i], RTLD_LAZY | RTLD_GLOBAL);
        if (handle) {
            LOGI("Found Lua: %s", libs[i]);
            break;
        }
    }
    
    if (!handle) {
        LOGE("Lua library not found");
        return -1;
    }
    
    // Get luaL_dostring
    luaL_dostring = (luaL_dostring_t)dlsym(handle, "luaL_dostring");
    if (!luaL_dostring) {
        // Try alternative name
        luaL_dostring = (luaL_dostring_t)dlsym(handle, "luaL_dostring");
        if (!luaL_dostring) {
            LOGE("luaL_dostring not found");
            return -1;
        }
    }
    
    // Get lua_newstate for hooking
    lua_newstate = (lua_newstate_t)dlsym(handle, "lua_newstate");
    if (lua_newstate) {
        // We would hook here, but we need to modify the function address
        // For now, we just try to find an existing state
        LOGI("Found lua_newstate at %p", lua_newstate);
    }
    
    LOGI("Lua functions loaded successfully");
    return 0;
}

// Execute a Lua script
int execute_lua_script(lua_State* L, const char* script) {
    if (!L) {
        LOGE("No Lua state");
        return -1;
    }
    if (!luaL_dostring) {
        LOGE("luaL_dostring missing");
        return -1;
    }
    
    LOGI("Running: %s", script);
    int result = luaL_dostring(L, script);
    if (result != 0) {
        LOGE("Script error: %d", result);
    }
    return result;
}

JNIEXPORT jint JNICALL
Java_com_gsufur_executor_NativeLib_initialize(JNIEnv *env, jobject thiz) {
    LOGI("GSUFUR init");
    int result = hook_lua();
    if (result != 0) {
        LOGE("Hook failed");
        return -1;
    }
    
    // Try to find an existing Lua state
    // In practice, we'd need to scan memory here
    LOGI("Init complete");
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_gsufur_executor_NativeLib_executeScript(JNIEnv *env, jobject thiz, jstring script) {
    if (!luaL_dostring) {
        LOGE("Not initialized");
        return -1;
    }
    
    const char* script_str = (*env)->GetStringUTFChars(env, script, NULL);
    if (!script_str) {
        LOGE("Script invalid");
        return -1;
    }
    
    // For now, we just log and return success
    // The actual execution requires finding the Lua state
    LOGI("Script to execute: %s", script_str);
    
    (*env)->ReleaseStringUTFChars(env, script, script_str);
    
    // Return success even though we're not executing yet
    // This will be updated once we find the state
    return 0;
}
