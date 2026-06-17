/*
Copyright (C) 2016-2024 Xander Electronics. All right reserved.

This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
*/

#ifndef _BASE64UTIL_H
#define _BASE64UTIL_H

class Base64Util{
  public:
    int encode(char *output, char *input, int inputLength);
    int encodeURL(char *output, char *input, int inputLength);
    int decode(char * output, char * input, int inputLength);
    int encodedLength(int plainLength);
    int decodedLength(char * input, int inputLength);

  private:
    inline void fromA3ToA4(unsigned char * A4, unsigned char * A3);
    inline void fromA4ToA3(unsigned char * A3, unsigned char * A4);
    inline unsigned char lookupTable(char c);
};
extern Base64Util base64Util;

#endif // _BASE64UTIL_H
