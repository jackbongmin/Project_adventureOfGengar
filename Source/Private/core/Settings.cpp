#include "core/Settings.h"

namespace gengar {

Settings& Prefs() {
  static Settings s;
  return s;
}

}  // namespace gengar
