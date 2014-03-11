#include <string>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/hmac.h>

std::string intToString(int number) {
  std::ostringstream ss;
  ss << number;
  return ss.str();
}

std::string hashToHex(const unsigned char *digest, int len) {
  char hexstring[1024] = { 0 };
  int i;
  for( i=0; i<len; i++ ){
    sprintf( &hexstring[2*i], "%02x", digest[i] );
  }
  return hexstring;
}

std::string hmac_sha1(const std::string& key,const std::string& data) {
  unsigned char* res;

  res = HMAC (EVP_sha1(), (const unsigned char*)key.c_str(), key.length(), (const unsigned char*)data.c_str(), data.length() , NULL, NULL);

  std::string result((char*)res);
  return result;
}

std::string md5(const std::string& text) {
  EVP_MD_CTX mdctx;
  unsigned char md_value[EVP_MAX_MD_SIZE];
  unsigned int md_len;

  EVP_DigestInit( &mdctx, EVP_md5() );
  EVP_DigestUpdate( &mdctx, text.c_str(), text.size() );
  EVP_DigestFinal_ex( &mdctx, md_value, &md_len );
  EVP_MD_CTX_cleanup( &mdctx );

  return hashToHex(md_value, md_len);
}

std::string rawurlencode(const std::string& text) {
  const std::string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_.~";
  std::ostringstream result;

  for(unsigned int i = 0; i < text.length(); i++) {
    if((charset.find(text[i])) == std::string::npos) {
      std::ostringstream os;
      os.setf(std::ios::hex, std::ios::basefield);
      os.setf(std::ios::uppercase);
      os << (int)text[i];

      std::string tmp = os.str();

      if(tmp.length() > 2)
        tmp.erase(0,6);

      result << "%" << tmp;
    }
    else {
      result << text[i];
    }
  }

  return result.str();
}

std::string base64_encode(std::string text){
  EVP_ENCODE_CTX ectx;
  int size = text.size()*2;
  size = size > 64 ? size : 64;
  unsigned char* out = (unsigned char*)malloc( size );
  int outlen = 0;
  int tlen = 0;

  EVP_EncodeInit( &ectx );
  EVP_EncodeUpdate( &ectx, out, &outlen, (const unsigned char*)text.c_str(), text.size() );
  tlen += outlen;
  EVP_EncodeFinal( &ectx, out+tlen, &outlen );
  tlen += outlen;

  std::string str( (char*)out, tlen );
  free( out );

  return str;
}
