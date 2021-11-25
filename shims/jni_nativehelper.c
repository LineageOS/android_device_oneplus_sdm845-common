#include <android/file_descriptor_jni.h>
#include <jni.h>

extern int jniGetFDFromFileDescriptor(JNIEnv* env, jobject fileDescriptor) {
    if (!fileDescriptor) {
        return -1;
    }
    return AFileDescriptor_getFd(env, fileDescriptor);
}
