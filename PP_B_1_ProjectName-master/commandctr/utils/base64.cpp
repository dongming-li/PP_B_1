#include "base64.h"

Base64::Base64() {
//nothing
}

inline bool Base64::is_valid64(unsigned char checkChar) {
   return ((checkChar == '+') || (checkChar == '/') || isalnum(checkChar));
}

std::string Base64::decode(std::string const& encoded_string) {
   int indexlen = encoded_string.size();
   int i = 0;
   int j = 0;
   int index = 0;
   unsigned char six_bit_array[4], eight_bit_array[3];
   std::string decodedString;

   while (indexlen-- && ( encoded_string[index] != '=') && is_valid64(encoded_string[index])) {
      six_bit_array[i++] = encoded_string[index]; index++;
      if (i == 4) {
         for (i = 0; i <4; i++){
            six_bit_array[i] = base64_chars.find(six_bit_array[i]);
         }

         eight_bit_array[0] = ( six_bit_array[0] << 2       ) + ((six_bit_array[1] & 0x30) >> 4);
         eight_bit_array[1] = ((six_bit_array[1] & 0xf) << 4) + ((six_bit_array[2] & 0x3c) >> 2);
         eight_bit_array[2] = ((six_bit_array[2] & 0x3) << 6) +   six_bit_array[3];

         for (i = 0; (i < 3); i++){
            decodedString += eight_bit_array[i];
         }
         i = 0;
      }
   }

   if (i) {
      for (j = 0; j < i; j++){
         six_bit_array[j] = base64_chars.find(six_bit_array[j]);
      }

      eight_bit_array[0] = (six_bit_array[0] << 2) + ((six_bit_array[1] & 0x30) >> 4);
      eight_bit_array[1] = ((six_bit_array[1] & 0xf) << 4) + ((six_bit_array[2] & 0x3c) >> 2);

      for (j = 0; (j < i - 1); j++){
         decodedString += eight_bit_array[j];
      }
   }
   return decodedString;
}

std::string Base64::encode(unsigned char const* bytes_to_encode, unsigned int indexlen) {
   std::string encodedString;
   int counter = 0;
   unsigned char eight_bit_array[3];
   unsigned char six_bit_array[4];

   //loop through all of the binary data
   while (indexlen--) {
      eight_bit_array[counter++] = *(bytes_to_encode++);

      if (counter == 3) {
         six_bit_array[0] = (eight_bit_array[0] & 0xfc) >> 2;
         six_bit_array[1] = ((eight_bit_array[0] & 0x03) << 4) + ((eight_bit_array[1] & 0xf0) >> 4);
         six_bit_array[2] = ((eight_bit_array[1] & 0x0f) << 2) + ((eight_bit_array[2] & 0xc0) >> 6);
         six_bit_array[3] = eight_bit_array[2] & 0x3f;

         for(int i = 0; (i <4) ; i++){
            encodedString += base64_chars[six_bit_array[i]];
         }
         counter = 0;
      }
   }

   if (counter != 0){
      for(int j = counter; j < 3; j++){
         eight_bit_array[j] = '\0';
      }

      six_bit_array[0] = ( eight_bit_array[0] & 0xfc) >> 2;
      six_bit_array[1] = ((eight_bit_array[0] & 0x03) << 4) + ((eight_bit_array[1] & 0xf0) >> 4);
      six_bit_array[2] = ((eight_bit_array[1] & 0x0f) << 2) + ((eight_bit_array[2] & 0xc0) >> 6);

      for (int j = 0; (j < counter + 1); j++){
         encodedString += base64_chars[six_bit_array[j]];
      }

      while(counter++ < 3){
         encodedString += '=';
      }
   }
   return encodedString;
}
