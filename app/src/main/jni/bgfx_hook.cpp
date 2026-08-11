#include <jni.h>
#include <string.h>
#include <dlfcn.h>
#include <android/log.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

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

uintptr_t get_library_base(const char* lib_name) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return 0;
    uintptr_t base = 0;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, lib_name)) {
            char* end = strchr(line, '-');
            if (end) { *end = '\0'; base = strtoull(line, NULL, 16); break; }
        }
    }
    fclose(fp);
    return base;
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
    uintptr_t base = get_library_base("libBlockMan.so");
    if (!base) { LOGE("Base not found"); return; }
    LOGI("Base: 0x%lx", base);
    // Offsets (64-bit)
    org_getglobal = (lua_getglobal_t)(base + 0x19630E0);
    org_pushstring = (lua_pushstring_t)(base + 0x1962DF8);
    org_pcall = (lua_pcall_t)(base + 0x1963C5C);
    if (org_getglobal && org_pushstring && org_pcall) {
        g_hooked = true;
        LOGI("Hooks installed");
    }
}

extern "C" {
JNIEXPORT jint JNICALL Java_com_gsufur_executor_NativeLib_initialize(JNIEnv* env, jobject thiz) {
    LOGI("GSUFUR init");
    inject();
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
