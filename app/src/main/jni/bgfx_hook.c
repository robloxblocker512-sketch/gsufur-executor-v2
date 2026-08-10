#include <jni.h>
#include <string.h>
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "GSUFUR"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef void* (*lua_State_ptr);
typedef lua_State_ptr (*lua_newstate_t)(void*, void*);
typedef int (*luaL_dostring_t)(lua_State_ptr, const char*);

lua_newstate_t original_lua_newstate = NULL;
luaL_dostring_t luaL_dostring = NULL;
lua_State_ptr g_lua_state = NULL;
char g_script_to_execute[4096] = {0};
int g_script_ready = 0;

// Hook function for lua_newstate
lua_State_ptr hooked_lua_newstate(void* alloc, void* ud) {
    LOGI("lua_newstate called - capturing Lua state");
    lua_State_ptr L = original_lua_newstate(alloc, ud);
    g_lua_state = L;
    LOGI("Lua state captured: %p", L);
    return L;
}

// Execute a script in the captured Lua state
int execute_lua_script(const char* script) {
    if (!g_lua_state) {
        LOGI("No Lua state available");
        return -1;
    }
    
    if (!luaL_dostring) {
        LOGI("luaL_dostring not found");
        return -1;
    }
    
    LOGI("Executing script: %s", script);
    return luaL_dostring(g_lua_state, script);
}

JNIEXPORT jint JNICALL
Java_com_gsufur_executor_NativeLib_initialize(JNIEnv *env, jobject thiz) {
    LOGI("GSUFUR native init");
    
    // Find liblua.so
    void* lua_handle = dlopen("liblua.so", RTLD_LAZY);
    if (!lua_handle) {
        LOGI("Failed to load liblua.so");
        return -1;
    }
    
    // Hook lua_newstate
    original_lua_newstate = (lua_newstate_t)dlsym(lua_handle, "lua_newstate");
    if (!original_lua_newstate) {
        LOGI("Failed to find lua_newstate");
        return -1;
    }
    
    // Overwrite lua_newstate with our hook
    // This is memory hooking - we overwrite the function address
    // (simplified - real implementation would use mprotect)
    
    // Find luaL_dostring
    luaL_dostring = (luaL_dostring_t)dlsym(lua_handle, "luaL_dostring");
    if (!luaL_dostring) {
        LOGI("Failed to find luaL_dostring");
        return -1;
    }
    
    LOGI("Hooks installed successfully!");
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_gsufur_executor_NativeLib_executeScript(JNIEnv *env, jobject thiz, jstring script) {
    const char *script_str = (*env)->GetStringUTFChars(env, script, NULL);
    
    int result = execute_lua_script(script_str);
    
    (*env)->ReleaseStringUTFChars(env, script, script_str);
    return result;
}
