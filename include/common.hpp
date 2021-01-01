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

/** real number value */
typedef double Real;
// typedef float Real;
/** power value type for spectral distributions */
typedef Real Power;
// typedef unsigned int WaveLength;
/** Wave length type for spectral distributions */
typedef float WaveLength;
/** indexing type for spectral distributions */
typedef WaveLength Index;

/** short hand for unsigned int */
typedef unsigned int uint;

/** 2d spectral distribution value with its index and power
 * value */
typedef std::pair<WaveLength, Power> WavePower;
}
