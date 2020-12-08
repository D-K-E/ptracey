#ifndef SPBREADER_HPP
#define SPBREADER_HPP
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
#include <string>
//
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

/**
  Check if file header is "SPB"
 */
bool check_file_identifier(char headerInfo[HEADER_SIZE]) {
  return headerInfo[0] == 'S' && headerInfo[1] == 'P' &&
         headerInfo[2] == 'B';
}

void get_header(std::ifstream &file,
                char headerInfo[HEADER_SIZE]) {
  //
  // set file to back
  if (file.is_open()) {
    file.seekg(0);
    file.read(headerInfo, HEADER_SIZE);
  } else {
    throw std::runtime_error("spb file is not opened");
  }
  if (!check_file_identifier(headerInfo)) {
    std::stringstream ss;
    ss << "File identifier for spb file is not correct."
       << std::endl
       << "It should be 'SPB'"
       << "see given header: " << headerInfo << std::endl;
    std::string err = ss.str();
    throw std::runtime_error(err);
  }
}
void get_sub(unsigned int &start, char arr[4],
             char headerInfo[HEADER_SIZE]) {
  for (unsigned int k = 0; k < 4; k++) {
    arr[k] = headerInfo[start];
    start++;
  }
}
void get_sub_ui(unsigned int &start, uint32_t &val,
                char headerInfo[HEADER_SIZE]) {
  char arr[4];
  get_sub(start, arr, headerInfo);
  val = *(uint32_t *)(arr);
}
void get_sub_f(unsigned int &start, float &val,
               char headerInfo[HEADER_SIZE]) {
  char arr[4];
  get_sub(start, arr, headerInfo);
  val = *(float *)(arr);
}
void parseFileHeader(char headerInfo[HEADER_SIZE],
                     uint32_t &width, uint32_t &height,
                     uint32_t &nb_channels,
                     float &first_wavelength,
                     float &wavelength_resolution,
                     float &last_wavelength) {
  unsigned int start = 3;
  get_sub_ui(start, width, headerInfo);
  get_sub_ui(start, height, headerInfo);
  get_sub_ui(start, nb_channels, headerInfo);
  get_sub_f(start, first_wavelength, headerInfo);
  get_sub_f(start, wavelength_resolution, headerInfo);
  get_sub_f(start, last_wavelength, headerInfo);
}
void getImageData(std::ifstream &file, float *data,
                  const uint32_t &width,
                  const uint32_t &height,
                  const uint32_t &nb_channels) {
  file.seekg(0);
  file.seekg(HEADER_SIZE, file.end);
  int data_length = file.tellg();
  char *buffer = new char[data_length];
  file.read(buffer, data_length);
  //
  uint32_t stride = 4;
  uint32_t total_size = width * height * nb_channels;
  //
  for (uint32_t pos = 0; pos < total_size; pos++) {
    char arr[stride];
    for (int i = 0; i < stride; i++) {
      arr[i] = buffer[pos + i];
    }
    data[pos] = *(float *)arr;
  }
  delete[] buffer;
}

void read_header(const char *fpath, uint32_t &width,
                 uint32_t &height, uint32_t &nb_channels,
                 float &first_wavelength,
                 float &wavelength_resolution,
                 float &last_wavelength) {
  std::ifstream file;
  file.open(fpath, std::ios::in | std::ios::binary);
  char headerInfo[HEADER_SIZE];
  get_header(file, headerInfo);
  //
  parseFileHeader(headerInfo, width, height, nb_channels,
                  first_wavelength, wavelength_resolution,
                  last_wavelength);
  file.close();
}

void read_file(const char *fpath, const uint32_t &width,
               const uint32_t &height,
               const uint32_t &nb_channels, float *data) {
  //
  std::ifstream file;
  file.open(fpath, std::ios::in | std::ios::binary);
  getImageData(file, data, width, height, nb_channels);
  file.close();
}
}
#endif
