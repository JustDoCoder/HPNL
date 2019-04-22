#include "HPNL/ExternalCqService.h"
#include "HPNL/ExternalEqService.h"

#include "com_intel_hpnl_core_CqService.h"

static jmethodID handleCallback;

static jfieldID _get_self_id(JNIEnv *env, jobject thisObj)
{
  static int init = 0;
  static jfieldID fidSelfPtr;
  if(!init)
  {
    jclass thisClass = env->GetObjectClass(thisObj);
    fidSelfPtr = env->GetFieldID(thisClass, "nativeHandle", "J");
    jclass connClass = env->FindClass("com/intel/hpnl/core/Connection");
    handleCallback = (*env).GetMethodID(connClass, "handleCallback", "(III)I");

    init = 1;
  }
  return fidSelfPtr;
}

static ExternalCqService*_get_self(JNIEnv *env, jobject thisObj)
{
  jlong selfPtr = env->GetLongField(thisObj, _get_self_id(env, thisObj));
  return *(ExternalCqService**)&selfPtr;
}

static void _set_self(JNIEnv *env, jobject thisObj, ExternalCqService *self)
{
  jlong selfPtr = *(jlong*)&self;
  env->SetLongField(thisObj, _get_self_id(env, thisObj), selfPtr);
}

/*
 * Class:     com_intel_hpnl_core_CqService
 * Method:    wait_cq_event
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_CqService_wait_1cq_1event(JNIEnv *env, jobject thisObj, jint index, jlong cqServicePtr) {
  ExternalCqService *externalCqService = *(ExternalCqService **)&cqServicePtr;
  fid_eq *eq;
  Chunk *chunk;
  int block_buffer_size = 0;
  int rdma_buffer_id = 0;
  int ret = externalCqService->wait_cq_event(index, &eq, &chunk, &rdma_buffer_id, &block_buffer_size);
  if (ret <= 0) {
    return ret; 
  }
  FIConnection* conn = (FIConnection*)chunk->con;
  if (!conn)
    return -1;

  jint rst = (*env).CallIntMethod(thisObj, handleCallback, ret, rdma_buffer_id, block_buffer_size);
  if (ret == RECV_EVENT && rst) {
	Chunk *ck = externalCqService->get_chunk(rdma_buffer_id, RECV_CHUNK);
	if (conn->activate_chunk(ck)) {
		perror("failed to return receive chunk/buffer");
	}
  }
  return ret;
}

/*
 * Class:     com_intel_hpnl_core_CqService
 * Method:    init
 * Signature: (J)V
 */
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_CqService_init(JNIEnv *env, jobject thisObj, jlong eqServicePtr) {
  ExternalEqService *externalEqService = *(ExternalEqService **)&eqServicePtr;
  ExternalCqService *externalCqService = new ExternalCqService(externalEqService, externalEqService->get_stack());
  _set_self(env, thisObj, externalCqService);
  return externalCqService->init();
}

/*
 * Class:     com_intel_hpnl_core_CqService
 * Method:    finalize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_CqService_finalize(JNIEnv *env, jobject thisObj) {
  ExternalCqService *externalCqService = _get_self(env, thisObj);
  if (externalCqService != NULL) {
    delete externalCqService;
    externalCqService = NULL;
    _set_self(env, thisObj, NULL);
  }
}

/*
 * Class:     com_intel_hpnl_core_CqService
 * Method:    free
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_CqService_free(JNIEnv *env, jobject thisObj, jlong cqServicePtr) {
  ExternalCqService *externalCqService = *(ExternalCqService **)&cqServicePtr;
  if (externalCqService != NULL) {
    delete externalCqService;
    externalCqService = NULL;
    _set_self(env, thisObj, NULL);
  }
}

