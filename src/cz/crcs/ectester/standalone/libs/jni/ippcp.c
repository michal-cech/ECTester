#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "native.h"

#include <ippcp.h>

#include "c_timing.h"
#include "c_utils.h"

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <time.h>

#define USE_SPEEDUP 1

static IppsPRNGState *prng_state;
static jclass provider_class;

/* This needs to be specified in this way because ippcp does not offer functionality to retrieve
   information about supported curves in any way. */
typedef struct {
	const char name[128];
	IppECCType id;
	int size;
	IppStatus (*context_size_func)(int *);
	IppStatus (*init_func)(IppsECCPState *);
	IppStatus (*set_func)(IppsECCPState *);
	IppStatus (*precomp_func)(IppsECCPState *);
} ippcp_curve;

static const ippcp_curve CURVES[] = {
    {"secp112r1", IppECCPStd112r1, 112, NULL, NULL, NULL, NULL},
    {"secp112r2", IppECCPStd112r2, 112, NULL, NULL, NULL, NULL},
    {"secp128r1", IppECCPStd128r1, 128, ippsECCPGetSizeStd128r1, ippsECCPInitStd128r1, ippsECCPSetStd128r1, NULL},
    {"secp128r2", IppECCPStd128r2, 128, ippsECCPGetSizeStd128r2, ippsECCPInitStd128r2, ippsECCPSetStd128r2, NULL},
    {"secp160r1", IppECCPStd160r1, 160, NULL, NULL, NULL, NULL},
    {"secp160r2", IppECCPStd160r2, 160, NULL, NULL, NULL, NULL},
    {"secp192r1", IppECCPStd192r1, 192, ippsECCPGetSizeStd192r1, ippsECCPInitStd192r1, ippsECCPSetStd192r1, ippsECCPBindGxyTblStd192r1},
    {"secp224r1", IppECCPStd224r1, 224, ippsECCPGetSizeStd224r1, ippsECCPInitStd224r1, ippsECCPSetStd224r1, ippsECCPBindGxyTblStd224r1},
    {"secp256r1", IppECCPStd256r1, 256, ippsECCPGetSizeStd256r1, ippsECCPInitStd256r1, ippsECCPSetStd256r1, ippsECCPBindGxyTblStd256r1},
    {"secp384r1", IppECCPStd384r1, 384, ippsECCPGetSizeStd384r1, ippsECCPInitStd384r1, ippsECCPSetStd384r1, ippsECCPBindGxyTblStd384r1},
    {"secp521r1", IppECCPStd521r1, 521, ippsECCPGetSizeStd521r1, ippsECCPInitStd521r1, ippsECCPSetStd521r1, ippsECCPBindGxyTblStd521r1}};

static const int NUM_CURVES = sizeof(CURVES) / sizeof(ippcp_curve);

JNIEXPORT jobject JNICALL Java_cz_crcs_ectester_standalone_libs_IppcpLib_createProvider(JNIEnv *env, jobject this) {
	/* Create the custom provider. */
	jclass local_provider_class = (*env)->FindClass(env, "cz/crcs/ectester/standalone/libs/jni/NativeProvider$Ippcp");
	provider_class = (*env)->NewGlobalRef(env, local_provider_class);

	jmethodID init = (*env)->GetMethodID(env, local_provider_class, "<init>", "(Ljava/lang/String;DLjava/lang/String;)V");

	const IppLibraryVersion *lib = ippcpGetLibVersion();
	jstring name = (*env)->NewStringUTF(env, lib->Name);
	double version = (double)lib->major + ((double)lib->minor / 10);
	jstring info = (*env)->NewStringUTF(env, lib->Version);

	// printf("%s\n%s\n%d.%d.%d.%d\n", lib->Name, lib->Version, lib->major, lib->minor, lib->majorBuild, lib->build);

	return (*env)->NewObject(env, provider_class, init, name, version, info);
}

JNIEXPORT void JNICALL Java_cz_crcs_ectester_standalone_libs_jni_NativeProvider_00024Ippcp_setup(JNIEnv *env, jobject this) {
	INIT_PROVIDER(env, provider_class);

	ADD_KPG(env, this, "EC", "Ippcp");
	// ADD_KA(env, self, "ECDH", "IppcplECDH");
	// ADD_SIG(env, self, "NONEwithECDSA", "IppcpECDSAwithNONE");

	/* Init the PRNG. */
	int prng_size;
	ippsPRNGGetSize(&prng_size);
	prng_state = malloc(prng_size);
	ippsPRNGInit(160, prng_state);
	/* We need to manually seed the PRNG, let's hope that everyone using ippcp does this.
	   Otherwise: nonce reuse in ECDSA, whoops! */
	int seed_len = 8;
	Ipp32u seed[seed_len];
	IppStatus res = ippsTRNGenRDSEED(seed, sizeof(seed) * 8, NULL);
	if (res != ippStsNoErr) {
		res = ippsPRNGenRDRAND(seed, sizeof(seed) * 8, NULL);
	}
	if (res != ippStsNoErr) {
		FILE *urandom = fopen("/dev/urandom", "rb");
		if (urandom) {
			size_t read = 0;
			while (read < sizeof(seed)) {
				read += fread(((uint8_t *)&seed) + read, 1, sizeof(seed) - read, urandom);
			}
			fclose(urandom);
			res = ippStsNoErr;
		}
	}
	if (res != ippStsNoErr) {
		struct timespec t;
		if (!clock_gettime(CLOCK_REALTIME, &t)) {
			memcpy(seed, &t.tv_nsec, sizeof(t.tv_nsec) > sizeof(seed) ? sizeof(seed) : sizeof(t.tv_nsec));
		} else {
			time_t tim = time(NULL);
			memcpy(seed, &tim, sizeof(time_t) > sizeof(seed) ? sizeof(seed) : sizeof(time_t));
		}
	}
	int bn_size;
	ippsBigNumGetSize(seed_len, &bn_size);
	uint8_t bn_buf[bn_size];
	IppsBigNumState *bn = (IppsBigNumState *)bn_buf;
	ippsBigNumInit(seed_len, bn);
	ippsSet_BN(IppsBigNumPOS, seed_len, seed, bn);
	ippsPRNGSetSeed(bn, prng_state);

	init_classes(env, "Ippcp");
}

static IppStatus prng_wrapper(Ipp32u *pRand, int nBits, void *pCtx) {
	native_timing_pause();
	IppStatus result = ippsPRNGen(pRand, nBits, pCtx);
	native_timing_restart();
	return result;
}

JNIEXPORT jobject JNICALL Java_cz_crcs_ectester_standalone_libs_IppcpLib_getCurves(JNIEnv *env, jobject this) {
	jclass hash_set_class = (*env)->FindClass(env, "java/util/TreeSet");

	jmethodID hash_set_ctr = (*env)->GetMethodID(env, hash_set_class, "<init>", "()V");
	jmethodID hash_set_add = (*env)->GetMethodID(env, hash_set_class, "add", "(Ljava/lang/Object;)Z");

	jobject result = (*env)->NewObject(env, hash_set_class, hash_set_ctr);

	for (size_t i = 0; i < NUM_CURVES; ++i) {
		jstring curve_name = (*env)->NewStringUTF(env, CURVES[i].name);
		(*env)->CallBooleanMethod(env, result, hash_set_add, curve_name);
	}
	return result;
}

JNIEXPORT jboolean JNICALL Java_cz_crcs_ectester_standalone_libs_jni_NativeKeyPairGeneratorSpi_00024Ippcp_keysizeSupported(JNIEnv *env,
                                                                                                                           jobject this,
                                                                                                                           jint keysize) {
	for (size_t i = 0; i < NUM_CURVES; ++i) {
		if (CURVES[i].size == keysize) {
			return JNI_TRUE;
		}
	}
	return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_cz_crcs_ectester_standalone_libs_jni_NativeKeyPairGeneratorSpi_00024Ippcp_paramsSupported(JNIEnv *env,
                                                                                                                          jobject this,
                                                                                                                          jobject params) {
	if (params == NULL) {
		return JNI_FALSE;
	}

	if ((*env)->IsInstanceOf(env, params, ec_parameter_spec_class)) {
		jmethodID get_curve = (*env)->GetMethodID(env, ec_parameter_spec_class, "getCurve", "()Ljava/security/spec/EllipticCurve;");
		jobject curve = (*env)->CallObjectMethod(env, params, get_curve);

		jmethodID get_field = (*env)->GetMethodID(env, elliptic_curve_class, "getField", "()Ljava/security/spec/ECField;");
		jobject field = (*env)->CallObjectMethod(env, curve, get_field);
		if ((*env)->IsInstanceOf(env, field, f2m_field_class)) {
			return JNI_FALSE;
		}
		return JNI_TRUE;
	} else if ((*env)->IsInstanceOf(env, params, ecgen_parameter_spec_class)) {
		jmethodID get_name = (*env)->GetMethodID(env, ecgen_parameter_spec_class, "getName", "()Ljava/lang/String;");
		jstring name = (*env)->CallObjectMethod(env, params, get_name);
		const char *utf_name = (*env)->GetStringUTFChars(env, name, NULL);
		for (size_t i = 0; i < NUM_CURVES; ++i) {
			if (strcasecmp(utf_name, CURVES[i].name) == 0) {
				(*env)->ReleaseStringUTFChars(env, name, utf_name);
				return JNI_TRUE;
			}
		}
		(*env)->ReleaseStringUTFChars(env, name, utf_name);
		return JNI_FALSE;
	} else {
		return JNI_FALSE;
	}
}


static IppsECCPPointState *new_point(int size) {
	int point_size;
	ippsECCPPointGetSize(size, &point_size);
	IppsECCPPointState *point = malloc(point_size);
	ippsECCPPointInit(size, point);
	return point;
}

static IppsBigNumState *new_bn(int bits) {
	int bn_size;
	int len = ((bits + 7) / 8) / sizeof(Ipp32u);
	ippsBigNumGetSize(len, &bn_size);
	IppsBigNumState *bn = malloc(bn_size);
	ippsBigNumInit(len, bn);
	return bn;
}

static void bn_get(IppsBigNumState *bn, uint8_t *buf, int lsb) {
	int size;
	ippsGetSize_BN(bn, &size);
	size *= sizeof(Ipp32u);
	uint8_t data[size];
	ippsGetOctString_BN(data, size, bn);
	memcpy(buf, data + (size - lsb), lsb);
}

static jobject bn_to_biginteger(JNIEnv *env, const IppsBigNumState *bn) {
	jmethodID biginteger_init = (*env)->GetMethodID(env, biginteger_class, "<init>", "(I[B)V");
	int bn_size;
	ippsGetSize_BN(bn, &bn_size);
	bn_size *= sizeof(Ipp32u);
	jbyteArray bytes = (*env)->NewByteArray(env, bn_size);
	jbyte *data = (*env)->GetByteArrayElements(env, bytes, NULL);
	ippsGetOctString_BN(data, bn_size, bn);
	(*env)->ReleaseByteArrayElements(env, bytes, data, 0);
	jobject result = (*env)->NewObject(env, biginteger_class, biginteger_init, 1, bytes);
	return result;
}

static IppsBigNumState *biginteger_to_bn(JNIEnv *env, jobject bigint) {
	jmethodID to_byte_array = (*env)->GetMethodID(env, biginteger_class, "toByteArray", "()[B");

    jbyteArray byte_array = (jbyteArray) (*env)->CallObjectMethod(env, bigint, to_byte_array);
    jsize byte_length = (*env)->GetArrayLength(env, byte_array);
    jbyte *byte_data = (*env)->GetByteArrayElements(env, byte_array, NULL);
	IppsBigNumState *result = new_bn(byte_length * 8);
	ippsSetOctString_BN(byte_data, byte_length, result);
	(*env)->ReleaseByteArrayElements(env, byte_array, byte_data, JNI_ABORT);
	return result;
}

static void biginteger_print(JNIEnv *env, jobject bigint) {
    jmethodID to_string = (*env)->GetMethodID(env, biginteger_class, "toString", "(I)Ljava/lang/String;");
    jstring big_string = (*env)->CallObjectMethod(env, bigint, to_string, (jint) 16);

    jsize len = (*env)->GetStringUTFLength(env, big_string);
    char raw_string[len + 1];
    raw_string[len] = 0;
    (*env)->GetStringUTFRegion(env, big_string, 0, len, raw_string);
    printf("%s\n", raw_string);
    fflush(stdout);
}

static IppsECCPState *create_curve(JNIEnv *env, jobject params, int *keysize) {
	jmethodID get_curve = (*env)->GetMethodID(env, ec_parameter_spec_class, "getCurve", "()Ljava/security/spec/EllipticCurve;");
    jobject curve = (*env)->CallObjectMethod(env, params, get_curve);

    jmethodID get_field = (*env)->GetMethodID(env, elliptic_curve_class, "getField", "()Ljava/security/spec/ECField;");
    jobject field = (*env)->CallObjectMethod(env, curve, get_field);

    jmethodID get_p = (*env)->GetMethodID(env, fp_field_class, "getP", "()Ljava/math/BigInteger;");
    jobject p = (*env)->CallObjectMethod(env, field, get_p);
	IppsBigNumState *p_bn = biginteger_to_bn(env, p);

	jmethodID get_a = (*env)->GetMethodID(env, elliptic_curve_class, "getA", "()Ljava/math/BigInteger;");
    jobject a = (*env)->CallObjectMethod(env, curve, get_a);
    IppsBigNumState *a_bn = biginteger_to_bn(env, a);

    jmethodID get_b = (*env)->GetMethodID(env, elliptic_curve_class, "getB", "()Ljava/math/BigInteger;");
    jobject b = (*env)->CallObjectMethod(env, curve, get_b);
    IppsBigNumState *b_bn = biginteger_to_bn(env, b);

	jmethodID get_g = (*env)->GetMethodID(env, ec_parameter_spec_class, "getGenerator", "()Ljava/security/spec/ECPoint;");
    jobject g = (*env)->CallObjectMethod(env, params, get_g);

    jmethodID get_x = (*env)->GetMethodID(env, point_class, "getAffineX", "()Ljava/math/BigInteger;");
    jobject gx = (*env)->CallObjectMethod(env, g, get_x);
    IppsBigNumState *gx_bn = biginteger_to_bn(env, gx);

    jmethodID get_y = (*env)->GetMethodID(env, point_class, "getAffineY", "()Ljava/math/BigInteger;");
    jobject gy = (*env)->CallObjectMethod(env, g, get_y);
    IppsBigNumState *gy_bn = biginteger_to_bn(env, gy);

	jmethodID get_n = (*env)->GetMethodID(env, ec_parameter_spec_class, "getOrder", "()Ljava/math/BigInteger;");
    jobject n = (*env)->CallObjectMethod(env, params, get_n);
	IppsBigNumState *n_bn = biginteger_to_bn(env, n);

	jmethodID get_h = (*env)->GetMethodID(env, ec_parameter_spec_class, "getCofactor", "()I");
	jint h = (*env)->CallIntMethod(env, params, get_h);

	jmethodID get_bitlength = (*env)->GetMethodID(env, biginteger_class, "bitLength", "()I");
	jint prime_bits = (*env)->CallIntMethod(env, p, get_bitlength);
	*keysize = prime_bits;

	int size;
	ippsECCPGetSize(prime_bits, &size);
	IppsECCPState *result = malloc(size);
	ippsECCPInit(prime_bits, result);
	ippsECCPSet(p_bn, a_bn, b_bn, gx_bn, gy_bn, n_bn, h, result);

	return result;
}

static jobject create_ec_param_spec(JNIEnv *env, int keysize, IppsECCPState *curve) {
	IppsBigNumState *p_bn = new_bn(keysize);
	IppsBigNumState *a_bn = new_bn(keysize);
	IppsBigNumState *b_bn = new_bn(keysize);
	int ord_bits;
	ippsECCPGetOrderBitSize(&ord_bits, curve);
	IppsBigNumState *gx_bn = new_bn(ord_bits);
	IppsBigNumState *gy_bn = new_bn(ord_bits);
	IppsBigNumState *order_bn = new_bn(ord_bits);
	int cofactor;

	IppStatus err = ippsECCPGet(p_bn, a_bn, b_bn, gx_bn, gy_bn, order_bn, &cofactor, curve);
	
	jobject p = bn_to_biginteger(env, p_bn);
    jmethodID fp_field_init = (*env)->GetMethodID(env, fp_field_class, "<init>", "(Ljava/math/BigInteger;)V");
    jobject field = (*env)->NewObject(env, fp_field_class, fp_field_init, p);
	free(p_bn);
	
	jobject a = bn_to_biginteger(env, a_bn);
	jobject b = bn_to_biginteger(env, b_bn);
	free(a_bn);
	free(b_bn);

	jmethodID elliptic_curve_init = (*env)->GetMethodID(env, elliptic_curve_class, "<init>", "(Ljava/security/spec/ECField;Ljava/math/BigInteger;Ljava/math/BigInteger;)V");
    jobject elliptic_curve = (*env)->NewObject(env, elliptic_curve_class, elliptic_curve_init, field, a, b);

	jobject gx = bn_to_biginteger(env, gx_bn);
	jobject gy = bn_to_biginteger(env, gy_bn);
	jmethodID point_init = (*env)->GetMethodID(env, point_class, "<init>", "(Ljava/math/BigInteger;Ljava/math/BigInteger;)V");
    jobject g = (*env)->NewObject(env, point_class, point_init, gx, gy);
	free(gx_bn);
	free(gy_bn);

	jobject n = bn_to_biginteger(env, order_bn);
	free(order_bn);

	jmethodID ec_parameter_spec_init = (*env)->GetMethodID(env, ec_parameter_spec_class, "<init>", "(Ljava/security/spec/EllipticCurve;Ljava/security/spec/ECPoint;Ljava/math/BigInteger;I)V");
    return (*env)->NewObject(env, ec_parameter_spec_class, ec_parameter_spec_init, elliptic_curve, g, n, cofactor);
}

static jobject generate_from_curve(JNIEnv *env, int keysize, IppsECCPState *curve) {
	IppsECCPPointState *point = new_point(keysize);

	int ord_bits;
	ippsECCPGetOrderBitSize(&ord_bits, curve);
	int ord_bytes = (ord_bits + 7) / 8;
	IppsBigNumState *secret = new_bn(ord_bits);

	native_timing_start();
	IppStatus err = ippsECCPGenKeyPair(secret, point, curve, prng_wrapper, prng_state);
	native_timing_stop();

	if (err != ippStsNoErr) {
		throw_new(env, "java/security/GeneralSecurityException", ippcpGetStatusString(err));
		free(point);
		free(secret);
		return NULL;
	}

	int coord_bytes = (keysize + 7) / 8;
	IppsBigNumState *x = new_bn(keysize);
	IppsBigNumState *y = new_bn(keysize);

	ippsECCPGetPoint(x, y, point, curve);

	jbyteArray pub_bytes = (*env)->NewByteArray(env, 2 * coord_bytes + 1);
	jbyte *pub_data = (*env)->GetByteArrayElements(env, pub_bytes, NULL);
	pub_data[0] = 0x04;
	bn_get(x, pub_data + 1, coord_bytes);
	bn_get(y, pub_data + 1 + coord_bytes, coord_bytes);
	(*env)->ReleaseByteArrayElements(env, pub_bytes, pub_data, 0);

	jbyteArray priv_bytes = (*env)->NewByteArray(env, ord_bytes);
	jbyte *priv_data = (*env)->GetByteArrayElements(env, priv_bytes, NULL);
	bn_get(secret, priv_data, ord_bytes);
	(*env)->ReleaseByteArrayElements(env, priv_bytes, priv_data, 0);

	free(point);
	free(secret);
	free(x);
	free(y);

	jobject ec_param_spec = create_ec_param_spec(env, keysize, curve);

    jobject ec_pub_param_spec = (*env)->NewLocalRef(env, ec_param_spec);
    jmethodID ec_pub_init = (*env)->GetMethodID(env, pubkey_class, "<init>", "([BLjava/security/spec/ECParameterSpec;)V");
    jobject pubkey = (*env)->NewObject(env, pubkey_class, ec_pub_init, pub_bytes, ec_pub_param_spec);

    jobject ec_priv_param_spec = (*env)->NewLocalRef(env, ec_param_spec);
    jmethodID ec_priv_init = (*env)->GetMethodID(env, privkey_class, "<init>", "([BLjava/security/spec/ECParameterSpec;)V");
    jobject privkey = (*env)->NewObject(env, privkey_class, ec_priv_init, priv_bytes, ec_priv_param_spec);

    jmethodID keypair_init = (*env)->GetMethodID(env, keypair_class, "<init>", "(Ljava/security/PublicKey;Ljava/security/PrivateKey;)V");
    return (*env)->NewObject(env, keypair_class, keypair_init, pubkey, privkey);
}

static jobject generate_from_curve_info(JNIEnv *env, const ippcp_curve *curve_info) {
	int context_size;
	if (curve_info->context_size_func) {
		curve_info->context_size_func(&context_size);
	} else {
		ippsECCPGetSize(curve_info->size, &context_size);
	}
	uint8_t curve_buf[context_size];
	IppsECCPState *curve = (IppsECCPState *)curve_buf;
	if (curve_info->init_func) {
		curve_info->init_func(curve);
	} else {
		ippsECCPInit(curve_info->size, curve);
	}
	if (curve_info->set_func) {
		curve_info->set_func(curve);
	} else {
		ippsECCPSetStd(curve_info->id, curve);
	}
	if (USE_SPEEDUP && curve_info->precomp_func) {
		curve_info->precomp_func(curve);
	}
	return generate_from_curve(env, curve_info->size, curve);
}

JNIEXPORT jobject JNICALL
Java_cz_crcs_ectester_standalone_libs_jni_NativeKeyPairGeneratorSpi_00024Ippcp_generate__ILjava_security_SecureRandom_2(JNIEnv *env,
                                                                                                                        jobject this,
                                                                                                                        jint keysize,
                                                                                                                        jobject random) {
	for (size_t i = 0; i < NUM_CURVES; ++i) {
		if (CURVES[i].size == keysize) {
			return generate_from_curve_info(env, &CURVES[i]);
		}
	}
	return NULL;
}

JNIEXPORT jobject JNICALL
Java_cz_crcs_ectester_standalone_libs_jni_NativeKeyPairGeneratorSpi_00024Ippcp_generate__Ljava_security_spec_AlgorithmParameterSpec_2Ljava_security_SecureRandom_2(
    JNIEnv *env, jobject this, jobject params, jobject random) {

	if ((*env)->IsInstanceOf(env, params, ec_parameter_spec_class)) {
		int keysize;
		IppsECCPState *curve = create_curve(env, params, &keysize);
		jobject result = generate_from_curve(env, keysize, curve);
		free(curve);
		return result;
	} else if ((*env)->IsInstanceOf(env, params, ecgen_parameter_spec_class)) {
		jmethodID get_name = (*env)->GetMethodID(env, ecgen_parameter_spec_class, "getName", "()Ljava/lang/String;");
		jstring name = (*env)->CallObjectMethod(env, params, get_name);
		const char *utf_name = (*env)->GetStringUTFChars(env, name, NULL);
		const ippcp_curve *curve_info;
		for (size_t i = 0; i < NUM_CURVES; ++i) {
			if (strcasecmp(utf_name, CURVES[i].name) == 0) {
				curve_info = &CURVES[i];
				break;
			}
		}
		(*env)->ReleaseStringUTFChars(env, name, utf_name);
		return generate_from_curve_info(env, curve_info);
	} else {
		return NULL;
	}
}

JNIEXPORT jboolean JNICALL Java_cz_crcs_ectester_standalone_libs_IppcpLib_supportsNativeTiming(JNIEnv *env, jobject this) {
	return native_timing_supported();
}

JNIEXPORT jlong JNICALL Java_cz_crcs_ectester_standalone_libs_IppcpLib_getNativeTimingResolution(JNIEnv *env, jobject this) {
	return native_timing_resolution();
}

JNIEXPORT jlong JNICALL Java_cz_crcs_ectester_standalone_libs_IppcpLib_getLastNativeTiming(JNIEnv *env, jobject this) {
	return native_timing_last();
}