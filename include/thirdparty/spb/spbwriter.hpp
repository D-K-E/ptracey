#ifndef SPBWRITER_HPP
#define SPBWRITER_HPP

/**
MIT License

Copyright (c) 2020 D.Kaan Eraslan & Qm Auber

Permission is hereby granted, free of charge, to any person
obtaining a copy
of this software and associated documentation files (the
"Software"), to deal
in the Software without restriction, including without
limitation the rights
to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall
be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE
SOFTWARE.
 */

#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <string>

namespace spb {
/**
Spectral Binary File Format (.spb) Specification

Table 1: Structure of Spectral Binary File.

File section                  | Bytes
----------------------------- | ----------
File identifier ’SPB’         | 3
Header Part                   | 24
Image width                   | 4
Image height                  | 4
Number of spectral channels n | 4
First wavelength              | 4
Wavelength Resolution         | 4
Last Wavelength               | 4
Image Data                    | x*y*n*4

Image data is written to the file in column order and values
are stored in little endian form.
Dimensions (x,y and n) are stored in uint32-format
and wavelength values in float32-format. Spectral image
values are reflectance values stored as float32.

*/
#define HEADER_SIZE 27
#define NB_HEADER_BYTE 4

void put_header(char hinfo[HEADER_SIZE],
                uint32_t &start_pos, const char *put_arr,
                uint32_t size) {
  for (uint32_t i = 0; i < size; i++) {
    hinfo[start_pos] = put_arr[i];
    start_pos++;
  }
}
bool isLittleEndian() {
  unsigned int i = 1;
  char *c = (char *)&i;
  if (*c)
    return true;
  return false;
}
void convert_uint_to_bytes_little(uint32_t t,
                                  char bytes[4]) {
  // https://stackoverflow.com/a/3784478/7330813
  char *tchar = (char *)&t;
  for (int i = 0; i < 4; i++) {
    bytes[i] = tchar[i];
  }
}
void convert_uint_to_bytes_big(uint32_t t, char bytes[4]) {
  convert_uint_to_bytes_little(t, bytes);
  char last = bytes[3];
  bytes[3] = bytes[0];
  bytes[0] = last;
}
void convert_float_to_bytes_little(float t, char bytes[4]) {
  // https://stackoverflow.com/a/3784478/7330813
  char *tchar = (char *)&t;
  for (int i = 0; i < 4; i++) {
    bytes[i] = tchar[i];
  }
}
void convert_float_to_bytes_big(float t, char bytes[4]) {
  convert_uint_to_bytes_little(t, bytes);
  char last = bytes[3];
  bytes[3] = bytes[0];
  bytes[0] = last;
}

void write_header(char headerInfo[HEADER_SIZE],
                  uint32_t width, uint32_t height,
                  uint32_t nb_channels,
                  float first_wavelength,
                  float wavelength_resolution,
                  float last_wavelength) {
  //
  const char file_h[] = "SPB";
  uint32_t start = 0;
  put_header(headerInfo, start, file_h, 3);

  char ws[NB_HEADER_BYTE];
  char hs[NB_HEADER_BYTE];
  char nb_chs[NB_HEADER_BYTE];
  char first_ws[NB_HEADER_BYTE];
  char waveres[NB_HEADER_BYTE];
  char last_ws[NB_HEADER_BYTE];

  if (isLittleEndian()) {
    convert_uint_to_bytes_little(width, ws);
    convert_uint_to_bytes_little(height, hs);
    convert_uint_to_bytes_little(nb_channels, nb_chs);
    convert_float_to_bytes_little(first_wavelength,
                                  first_ws);
    convert_float_to_bytes_little(wavelength_resolution,
                                  waveres);
    convert_float_to_bytes_little(last_wavelength, last_ws);
  } else {
    convert_uint_to_bytes_big(width, ws);
    convert_uint_to_bytes_big(height, hs);
    convert_uint_to_bytes_big(nb_channels, nb_chs);
    convert_float_to_bytes_big(first_wavelength, first_ws);
    convert_float_to_bytes_big(wavelength_resolution,
                               waveres);
    convert_float_to_bytes_big(last_wavelength, last_ws);
  }
  put_header(headerInfo, start, ws, NB_HEADER_BYTE);
  put_header(headerInfo, start, hs, NB_HEADER_BYTE);
  put_header(headerInfo, start, nb_chs, NB_HEADER_BYTE);
  put_header(headerInfo, start, first_ws, NB_HEADER_BYTE);
  put_header(headerInfo, start, waveres, NB_HEADER_BYTE);
  put_header(headerInfo, start, last_ws, NB_HEADER_BYTE);
}
void write_data(char *spb_data, uint32_t imsize,
                float *data) {
  auto convfn = isLittleEndian()
                    ? convert_float_to_bytes_little
                    : convert_float_to_bytes_big;
  for (unsigned int i = 0; i < imsize; i++) {
    float d = data[i];
    char darr[NB_HEADER_BYTE];
    convfn(d, darr);
    unsigned int start = i * 4;
    for (unsigned int k = 0; k < 4; k++) {
      spb_data[start + k] = darr[k];
    }
  }
}

void write_file(const char *path, uint32_t width,
                uint32_t height, uint32_t nb_channels,
                float first_wavelength,
                float wavelength_resolution,
                float last_wavelength, float *data) {
  std::ofstream file;
  file.open(path, std::ios::binary | std::ios::out);
  char hinfo[HEADER_SIZE];
  write_header(hinfo, width, height, nb_channels,
               first_wavelength, wavelength_resolution,
               last_wavelength);
  int spb_size = width * height * nb_channels * 4;
  char *spb_data = new char[spb_size];
  uint32_t imsize = width * height * nb_channels;
  write_data(spb_data, imsize, data);
  int spb_file_size = spb_size + HEADER_SIZE;
  char *spb_file = new char[spb_file_size];
  for (int i = 0; i < HEADER_SIZE; i++) {
    spb_file[i] = hinfo[i];
  }
  for (int i = HEADER_SIZE; i < spb_size; i++) {
    spb_file[i] = spb_data[i];
  }
  file.write(spb_file, spb_file_size);
  delete[] spb_data;
  delete[] spb_file;
}
}

#endif
