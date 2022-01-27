import JSEncrypt from 'jsencrypt';
import md5 from 'md5';
/**
 * @method rsa加密
 * @param {String} data 需要加密的字符串
 * @param {String} publicKey 提供的公钥
 * @returns {String} 加密后的字符串
 */
function getRsaCode(data, publicKey) {
  const encryptStr = new JSEncrypt();
  encryptStr.setPublicKey(publicKey);
  return encryptStr.encrypt(data.toString());
}
/**
 * @method md5加密
 * @param {String} data 需要加密的字符串
 * @returns {String} 加密后的字符串
 */
function getMd5Code(data) {
  return md5(data.toString());
}
/**
 * @enum {String}
 */
const ENCRYPT_MODE = {
  MD5: 'md5',
  RSA: 'rsa'
};

/**
 * @method 加密方法
 * @param {String} data 需要加密的字符串
 * @param {ENCRYPT_MODE} encryptMode=ENCRYPT_MODE.MD5 加密算法,默认MD5
 * @returns {String} 加密后的字符串
 */
export default function(data, encryptMode = ENCRYPT_MODE.MD5) {
  const rsaPublicKey = `MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDIhAZgXuynqnX+lAGsr7jFXt9FTqAqv8ZKViHosJ005IfM/Mrf3nIN5rjNmyOB7vcUfD+EgggrhcJsMSViCQpZbXEninVxg8ErPghRbhNdm1YvhM3VevMYKV6Y5fntbqkkrumd9byDGOIWqlwSZFa4Im9hvdLlkG9el62NlnQdfwIDAQAB`;
  // 判断data数据
  if (!data) {
    return '';
  }
  switch (encryptMode) {
    case ENCRYPT_MODE.RSA:
      return getRsaCode(data, rsaPublicKey);
    case ENCRYPT_MODE.MD5:
      return getMd5Code(data);
    default:
      console.error('未知的加密方式!');
      return '';
  }
}
