#include <jni.h>
#include <string.h>
#include <dlfcn.h>
#include <android/log.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>

#define LOG_TAG "GSUFUR"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Lua types
typedef struct lua_State lua_State;
typedef int (*luaL_dostring_t)(lua_State* L, const char* str);

// Function pointers
luaL_dostring_t luaL_dostring = NULL;
lua_State* g_lua_state = NULL;

// Execute script in Lua
int execute_lua_script(const char* script) {
    if (!g_lua_state) {
        LOGE("No Lua state available");
        return -1;
    }
    if (!luaL_dostring) {
        LOGE("luaL_dostring not available");
        return -1;
    }
    
    LOGI("Executing: %s", script);
    return luaL_dostring(g_lua_state, script);
}

// Scan memory for Lua state
void* scan_for_lua_state() {
    // Search for lua_State in memory
    // We look for the signature of a valid Lua state
    void* addr = NULL;
    FILE* maps = fopen("/proc/self/maps", "r");
    if (!maps) return NULL;
    
    char line[512];
    while (fgets(line, sizeof(line), maps)) {
        unsigned long start, end;
        char perms[5];
        if (sscanf(line, "%lx-%lx %4s", &start, &end, perms) == 3) {
            if (perms[0] == 'r' && perms[1] == 'w') {
                // Search for Lua state signature
                for (unsigned long p = start; p < end - 8; p += 4) {
                    // Check for common Lua state patterns
                    unsigned long *ptr = (unsigned long*)p;
                    if (ptr[0] == 0x4C75615F && ptr[1] == 0x53746174) {
                        addr = (void*)p;
                        LOGI("Found potential Lua state at %p", addr);
                        break;
                    }
                }
            }
        }
        if (addr) break;
    }
    fclose(maps);
    return addr;
}

JNIEXPORT jint JNICALL
Java_com_gsufur_executor_NativeLib_initialize(JNIEnv *env, jobject thiz) {
    LOGI("GSUFUR native init");
    
    // Find liblua.so
    void* lua_handle = dlopen("liblua.so", RTLD_LAZY);
    if (!lua_handle) {
        // Try alternative names
        lua_handle = dlopen("liblua5.1.so", RTLD_LAZY);
        if (!lua_handle) {
            lua_handle = dlopen("libluajit.so", RTLD_LAZY);
        }
    }
    
    if (!lua_handle) {
        LOGE("Failed to find Lua library");
        return -1;
    }
    
    // Get luaL_dostring
    luaL_dostring = (luaL_dostring_t)dlsym(lua_handle, "luaL_dostring");
    if (!luaL_dostring) {
        LOGE("Failed to find luaL_dostring");
        return -1;
    }
    
    // Scan for Lua state
    g_lua_state = (lua_State*)scan_for_lua_state();
    if (!g_lua_state) {
        LOGE("Failed to find Lua state");
        return -1;
    }
    
    LOGI("Lua state found at %p", g_lua_state);
    LOGI("Initialization complete");
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_gsufur_executor_NativeLib_executeScript(JNIEnv *env, jobject thiz, jstring script) {
    const char *script_str = (*env)->GetStringUTFChars(env, script, NULL);
    if (!script_str) {
        LOGE("Failed to get script");
        return -1;
    }
    
    int result = execute_lua_script(script_str);
    
    (*env)->ReleaseStringUTFChars(env, script, script_str);
    return result;
}
