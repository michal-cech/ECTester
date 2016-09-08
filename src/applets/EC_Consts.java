/**
 * 
 */
package applets;

import javacard.framework.ISOException;
import javacard.framework.Util;
import javacard.security.ECPrivateKey;
import javacard.security.ECPublicKey;
import javacard.security.KeyBuilder;

public class EC_Consts {
    public static byte[] EC_FP_P = null;
    public static byte[] EC_FP_A = null;
    public static byte[] EC_FP_B = null;
    public static byte[] EC_FP_G_X = null;
    public static byte[] EC_FP_G_Y = null;
    public static byte[] EC_FP_R = null;
    public static short EC_FP_K = 1;
    
    // secp192r1 from http://www.secg.org/sec2-v2.pdf
    public static final byte[] EC192_FP_P = new byte[]{
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFE,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF};
    public static final byte[] EC192_FP_A = new byte[]{
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFE,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFC};
    public static final byte[] EC192_FP_B = new byte[]{
        (byte) 0x64, (byte) 0x21, (byte) 0x05, (byte) 0x19,
        (byte) 0xE5, (byte) 0x9C, (byte) 0x80, (byte) 0xE7,
        (byte) 0x0F, (byte) 0xA7, (byte) 0xE9, (byte) 0xAB,
        (byte) 0x72, (byte) 0x24, (byte) 0x30, (byte) 0x49,
        (byte) 0xFE, (byte) 0xB8, (byte) 0xDE, (byte) 0xEC,
        (byte) 0xC1, (byte) 0x46, (byte) 0xB9, (byte) 0xB1};
    // G in compressed form / first part of ucompressed
    public static final byte[] EC192_FP_G_X = new byte[]{
        (byte) 0x18, (byte) 0x8D, (byte) 0xA8, (byte) 0x0E,
        (byte) 0xB0, (byte) 0x30, (byte) 0x90, (byte) 0xF6,
        (byte) 0x7C, (byte) 0xBF, (byte) 0x20, (byte) 0xEB,
        (byte) 0x43, (byte) 0xA1, (byte) 0x88, (byte) 0x00,
        (byte) 0xF4, (byte) 0xFF, (byte) 0x0A, (byte) 0xFD,
        (byte) 0x82, (byte) 0xFF, (byte) 0x10, (byte) 0x12};
    // second part of G uncompressed
    public static final byte[] EC192_FP_G_Y = new byte[]{ 
        (byte) 0x07, (byte) 0x19, (byte) 0x2B, (byte) 0x95,
        (byte) 0xFF, (byte) 0xC8, (byte) 0xDA, (byte) 0x78,
        (byte) 0x63, (byte) 0x10, (byte) 0x11, (byte) 0xED,
        (byte) 0x6B, (byte) 0x24, (byte) 0xCD, (byte) 0xD5,
        (byte) 0x73, (byte) 0xF9, (byte) 0x77, (byte) 0xA1,
        (byte) 0x1E, (byte) 0x79, (byte) 0x48, (byte) 0x11};
    // Order of G
    public static final byte[] EC192_FP_R = new byte[]{
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0x99, (byte) 0xDE, (byte) 0xF8, (byte) 0x36,
        (byte) 0x14, (byte) 0x6B, (byte) 0xC9, (byte) 0xB1,
        (byte) 0xB4, (byte) 0xD2, (byte) 0x28, (byte) 0x31};
    // cofactor of G
    public static final short EC192_FP_K = 1;    
    
    // secp256r1 from http://www.secg.org/sec2-v2.pdf
    public static final byte[] EC256_FP_P = new byte[]{
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x01,
        (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00,
        (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00,
        (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF};
    public static final byte[] EC256_FP_A = new byte[]{
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x01,
        (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00,
        (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00,
        (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFC};
    public static final byte[] EC256_FP_B = new byte[]{
        (byte) 0x5A, (byte) 0xC6, (byte) 0x35, (byte) 0xD8,
        (byte) 0xAA, (byte) 0x3A, (byte) 0x93, (byte) 0xE7,
        (byte) 0xB3, (byte) 0xEB, (byte) 0xBD, (byte) 0x55,
        (byte) 0x76, (byte) 0x98, (byte) 0x86, (byte) 0xBC,
        (byte) 0x65, (byte) 0x1D, (byte) 0x06, (byte) 0xB0,
        (byte) 0xCC, (byte) 0x53, (byte) 0xB0, (byte) 0xF6,
        (byte) 0x3B, (byte) 0xCE, (byte) 0x3C, (byte) 0x3E,
        (byte) 0x27, (byte) 0xD2, (byte) 0x60, (byte) 0x4B};
    // G in compressed form / first part of ucompressed
    public static final byte[] EC256_FP_G_X = new byte[]{
        (byte) 0x6B, (byte) 0x17, (byte) 0xD1, (byte) 0xF2,
        (byte) 0xE1, (byte) 0x2C, (byte) 0x42, (byte) 0x47,
        (byte) 0xF8, (byte) 0xBC, (byte) 0xE6, (byte) 0xE5,
        (byte) 0x63, (byte) 0xA4, (byte) 0x40, (byte) 0xF2,
        (byte) 0x77, (byte) 0x03, (byte) 0x7D, (byte) 0x81,
        (byte) 0x2D, (byte) 0xEB, (byte) 0x33, (byte) 0xA0,
        (byte) 0xF4, (byte) 0xA1, (byte) 0x39, (byte) 0x45,
        (byte) 0xD8, (byte) 0x98, (byte) 0xC2, (byte) 0x96};
    // second part of G uncompressed
    public static final byte[] EC256_FP_G_Y = new byte[]{
        (byte) 0x4F, (byte) 0xE3, (byte) 0x42, (byte) 0xE2,
        (byte) 0xFE, (byte) 0x1A, (byte) 0x7F, (byte) 0x9B,
        (byte) 0x8E, (byte) 0xE7, (byte) 0xEB, (byte) 0x4A,
        (byte) 0x7C, (byte) 0x0F, (byte) 0x9E, (byte) 0x16,
        (byte) 0x2B, (byte) 0xCE, (byte) 0x33, (byte) 0x57,
        (byte) 0x6B, (byte) 0x31, (byte) 0x5E, (byte) 0xCE,
        (byte) 0xCB, (byte) 0xB6, (byte) 0x40, (byte) 0x68,
        (byte) 0x37, (byte) 0xBF, (byte) 0x51, (byte) 0xF5};
    // Order of G
    public static final byte[] EC256_FP_R = new byte[]{
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF,
        (byte) 0xBC, (byte) 0xE6, (byte) 0xFA, (byte) 0xAD,
        (byte) 0xA7, (byte) 0x17, (byte) 0x9E, (byte) 0x84,
        (byte) 0xF3, (byte) 0xB9, (byte) 0xCA, (byte) 0xC2,
        (byte) 0xFC, (byte) 0x63, (byte) 0x25, (byte) 0x51};
    // cofactor of G
    public static final short EC256_FP_K = 1;    
    
    // TODO: add parameters for longer lengths
    
    public static void setECKeyParams(ECPublicKey ecPubKey, ECPrivateKey ecPrivKey, short ecLength, byte[] auxBuffer) {
        // Select proper courve parameters
        switch (ecLength) {
            case (short) 192: {
                EC_FP_P = EC192_FP_P;
                EC_FP_A = EC192_FP_A;
                EC_FP_B = EC192_FP_B;
                EC_FP_G_X = EC192_FP_G_X;
                EC_FP_G_Y = EC192_FP_G_Y;
                EC_FP_R = EC192_FP_R;
                EC_FP_K = EC192_FP_K;     
                break;
            }
            case (short) 256: {
                EC_FP_P = EC256_FP_P;
                EC_FP_A = EC256_FP_A;
                EC_FP_B = EC256_FP_B;
                EC_FP_G_X = EC256_FP_G_X;
                EC_FP_G_Y = EC256_FP_G_Y;
                EC_FP_R = EC256_FP_R;
                EC_FP_K = EC256_FP_K;
                break;
            }            
            default: {
                ISOException.throwIt((short) -1);
            }
        }
        // prepare an ANSI X9.62 uncompressed EC point representation for G
        short gSize = (short) 1;
        gSize += (short) EC_FP_G_X.length;
        gSize += (short) EC_FP_G_Y.length;
        auxBuffer[0] = 0x04;
        short off = 1;
        off = Util.arrayCopy(EC_FP_G_X, (short) 0, auxBuffer, off, (short) EC_FP_G_X.length);
        Util.arrayCopy(EC_FP_G_Y, (short) 0, auxBuffer, off, (short) EC_FP_G_Y.length);

        // pre-set basic EC parameters:
        ecPubKey.setFieldFP(EC_FP_P, (short) 0, (short) EC_FP_P.length);
        ecPubKey.setA(EC_FP_A, (short) 0, (short) EC_FP_A.length);
        ecPubKey.setB(EC_FP_B, (short) 0, (short) EC_FP_B.length);
        ecPubKey.setG(auxBuffer, (short) 0, gSize);
        ecPubKey.setR(EC_FP_R, (short) 0, (short) EC_FP_R.length);
        ecPubKey.setK(EC_FP_K);
        
        ecPrivKey.setFieldFP(EC_FP_P, (short) 0, (short) EC_FP_P.length);
        ecPrivKey.setA(EC_FP_A, (short) 0, (short) EC_FP_A.length);
        ecPrivKey.setB(EC_FP_B, (short) 0, (short) EC_FP_B.length);
        ecPrivKey.setG(auxBuffer, (short) 0, gSize);
        ecPrivKey.setR(EC_FP_R, (short) 0, (short) EC_FP_R.length);
        ecPrivKey.setK(EC_FP_K);        
    }    
}
