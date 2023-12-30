#include <iostream>
#include <pthread.h>
#include <jnihook.h>

JavaVM *jvm;
jclass MinecraftClass;
jmethodID startAttackMethod;
jmethodID runTickMethod;

jvalue hkStartAttack(JNIEnv *jni, jmethodID callableMethod, jvalue *args, size_t nargs, void *arg)
{
	std::cout << "[MCH] Minecraft::startAttack called!" << std::endl;

	std::cout << "[MCH] Calling original Minecraft::startAttack..." << std::endl;
	jboolean result = jni->CallBooleanMethod((jobject)&args[0].l, callableMethod);
	std::cout << "[MCH] Original call result: " << (int)result << std::endl;

	return jvalue { .z = result };
}

jvalue hkRunTick(JNIEnv *jni, jmethodID callableMethod, jvalue *args, size_t nargs, void *arg)
{
	std::cout << "[MCH] Minecraft::runTick called!" << std::endl;
	std::cout << "[MCH] args[0] (thisptr) -> " << (void *)args[0].l << std::endl;
	std::cout << "[MCH] args[1] (boolean) -> " << (int)args[1].z << std::endl;

	std::cout << "[MCH] Calling original Minecraft::runTick..." << std::endl;
	jni->CallVoidMethod((jobject)&args[0].l, callableMethod, args[1].z);
	std::cout << "[MCH] Original Minecraft::runTick called" << std::endl;

	return jvalue { 0 };
}

static void setup(JNIEnv *jni)
{
	MinecraftClass = jni->FindClass("evi"); // net.minecraft.client.Minecraft
	std::cout << "[MCH] Minecraft class: " << MinecraftClass << std::endl;

	startAttackMethod = jni->GetMethodID(MinecraftClass, "bo", "()Z"); // boolean startAttack()
	std::cout << "[MCH] Minecraft::startAttack method: " << startAttackMethod << std::endl;

	runTickMethod = jni->GetMethodID(MinecraftClass, "d", "(Z)V");
	std::cout << "[MCH] Minecraft::runTick method: " << runTickMethod << std::endl;

	JNIHook_Init(jvm);
	JNIHook_Attach(startAttackMethod, hkStartAttack, NULL);
	JNIHook_Attach(runTickMethod, hkRunTick, NULL);
}

static void *main_thread(void *arg)
{
	JNIEnv *jni;

	freopen("/tmp/mchook.log", "w", stdout);

	std::cout << "[MCH] Library loaded!" << std::endl;

	if (JNI_GetCreatedJavaVMs(&jvm, 1, NULL) != JNI_OK) {
		std::cout << "[MCH] Failed to retrieve JavaVM pointer" << std::endl;
		return NULL;
	}

	std::cout << "[MCH] JavaVM: " << jvm << std::endl;

	if (jvm->AttachCurrentThread((void **)&jni, NULL) != JNI_OK) {
		std::cout << "[MCH] Failed to retrieve JNI environment" << std::endl;
		return NULL;
	}

	std::cout << "[MCH] JNIEnv: " << jni << std::endl;

	setup(jni);

	return arg;
}

void __attribute__((constructor))
dl_entry()
{
	pthread_t th;
	pthread_create(&th, NULL, main_thread, NULL);
}
