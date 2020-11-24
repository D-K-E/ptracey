#pragma once
// common library functions and type definitions
#include <external.hpp>

using std::sqrt;
using std::fabs;
using std::shared_ptr;
using std::make_shared;
using std::sqrt;
using std::make_pair;
using std::isnan;
using namespace std::filesystem;

namespace ptracey {
path RUNTIME_PATH = current_path();

typedef double Real;
// typedef float Real;
typedef Real Power;
// typedef unsigned int WaveLength;
typedef float WaveLength;
typedef WaveLength Index;
typedef unsigned int uint;
typedef std::pair<WaveLength, Power> WavePower;
}
