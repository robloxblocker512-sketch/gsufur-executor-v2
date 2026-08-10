package com.gsufur.executor;

public class NativeLib {
    static {
        System.loadLibrary("bgfx_hook");
    }
    public static native int initialize();
    public static native int executeScript(String script);
}
