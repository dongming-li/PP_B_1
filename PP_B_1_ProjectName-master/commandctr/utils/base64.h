#include <iostream>
#ifndef BASE64_H
#define BASE64_H

class Base64
{
private:
   const std::string base64_chars =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789+/";

    /*
       Checks whether or input is a valid base64 character
    */
   inline bool is_valid64(unsigned char checkChar);

public:
   Base64();
   /*
      Takes a base64 encoded string as input and decodes it
      encodedStringurns succesfully decoded string
   */
   std::string decode(std::string const& encoded_string);

   /*
      Takes an array of binary data and converts it into base64 encoded string
      returns base64 encoded string
      Steps:
      1. Read in first 3 bytes/24bits of data
      2. split the 24bits into 4 6bit sections
      3. convert each 6bit section into a base64 character
      4. if number of bytes not divisible by 3 pad with 0s
   */
   std::string encode(unsigned char const* bytes_to_encode, unsigned int indexlen);
};
#endif /* BASE64_H */
