#include <jni.h>
#include <string.h>
#include <dlfcn.h>
#include <android/log.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <thread>

#define LOG_TAG "GSUFUR"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

typedef struct lua_State lua_State;
typedef int (*lua_getglobal_t)(lua_State* L, const char* name);
typedef void (*lua_pushstring_t)(lua_State* L, const char* s);
typedef int (*lua_pcall_t)(lua_State* L, int nargs, int nresults, int errfunc);

lua_getglobal_t org_getglobal = NULL;
lua_pushstring_t org_pushstring = NULL;
lua_pcall_t org_pcall = NULL;
lua_State* g_lua_state = NULL;
bool g_hooked = false;

int hook_getglobal(lua_State* L, const char* name) {
    if (L != NULL) { g_lua_state = L; LOGI("Lua state: %p", L); }
    return org_getglobal ? org_getglobal(L, name) : 0;
}

void hook_pushstring(lua_State* L, const char* str) {
    if (L != NULL) { g_lua_state = L; }
    if (org_pushstring) org_pushstring(L, str);
}

int hook_pcall(lua_State* L, int nargs, int nresults, int errfunc) {
    if (L != NULL) { g_lua_state = L; }
    return org_pcall ? org_pcall(L, nargs, nresults, errfunc) : 0;
}

int execute_lua_script(const char* script) {
    if (!g_lua_state || !org_getglobal || !org_pushstring || !org_pcall) {
        LOGE("Not ready");
        return -1;
    }
    LOGI("Executing: %s", script);
    org_getglobal(g_lua_state, "loadstring");
    org_pushstring(g_lua_state, script);
    if (org_pcall(g_lua_state, 1, 1, 0) != 0) return -1;
    if (org_pcall(g_lua_state, 0, 0, 0) != 0) return -1;
    return 0;
}

bool is_library_loaded(const char* lib_name) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return false;
    bool found = false;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, lib_name)) { found = true; break; }
    }
    fclose(fp);
    return found;
}

void inject() {
    LOGI("Waiting for libBlockMan.so...");
    while (!is_library_loaded("libBlockMan.so")) sleep(1);
    
    LOGI("Loading Lua functions with dlopen/dlsym...");
    
    // Try different Lua library names
    const char* libs[] = {"liblua.so", "liblua5.1.so", "liblua5.2.so", "liblua5.3.so", "libluajit.so", NULL};
    void* handle = NULL;
    
    for (int i = 0; libs[i] != NULL; i++) {
        handle = dlopen(libs[i], RTLD_LAZY);
        if (handle) {
            LOGI("Found: %s", libs[i]);
            break;
        }
    }
    
    if (!handle) {
        LOGE("No Lua library found");
        return;
    }
    
    // Get function addresses using dlsym
    org_getglobal = (lua_getglobal_t)dlsym(handle, "lua_getglobal");
    org_pushstring = (lua_pushstring_t)dlsym(handle, "lua_pushstring");
    org_pcall = (lua_pcall_t)dlsym(handle, "lua_pcall");
    
    if (org_getglobal && org_pushstring && org_pcall) {
        g_hooked = true;
        LOGI("Lua functions loaded successfully!");
    } else {
        LOGE("Failed to load Lua functions");
    }
}

extern "C" {
JNIEXPORT jint JNICALL Java_com_gsufur_executor_NativeLib_initialize(JNIEnv* env, jobject thiz) {
    LOGI("GSUFUR init");
    std::thread([] { inject(); }).detach();
    return 0;
}
JNIEXPORT jint JNICALL Java_com_gsufur_executor_NativeLib_executeScript(JNIEnv* env, jobject thiz, jstring script) {
    if (!g_hooked) { LOGE("Not hooked"); return -1; }
    const char* s = env->GetStringUTFChars(script, NULL);
    int r = execute_lua_script(s);
    env->ReleaseStringUTFChars(script, s);
    return r;
}
}
