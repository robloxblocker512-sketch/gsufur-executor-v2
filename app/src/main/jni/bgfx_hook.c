#include <jni.h>
#include <string.h>
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "GSUFUR"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef void (*render_frame_t)(void*);
render_frame_t original_render_frame = NULL;

JNIEXPORT jint JNICALL
Java_com_gsufur_executor_NativeLib_initialize(JNIEnv *env, jobject thiz) {
    LOGI("GSUFUR native init");
    void* bgfx_handle = dlopen("libbgfx.so", RTLD_LAZY);
    if (!bgfx_handle) {
        LOGI("Failed to load bgfx");
        return -1;
    }
    void* render_fn = dlsym(bgfx_handle, "_Z15bgfx_render_framev");
    if (!render_fn) {
        LOGI("Failed to find render function");
        return -1;
    }
    original_render_frame = (render_frame_t)render_fn;
    LOGI("Hook installed successfully!");
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_gsufur_executor_NativeLib_executeScript(JNIEnv *env, jobject thiz, jstring script) {
    const char *script_str = (*env)->GetStringUTFChars(env, script, NULL);
    LOGI("Executing script: %s", script_str);
    (*env)->ReleaseStringUTFChars(env, script, script_str);
    return 0;
}
