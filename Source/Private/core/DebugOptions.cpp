#include "core/DebugOptions.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace gengar {
namespace {

DebugOptions g_options;

bool Truthy(const std::string& v) { return v.empty() || (v != "0" && v != "false"); }

void Apply(const std::string& key, const std::string& value) {
  if (key == "stage") g_options.startStage = std::atoi(value.c_str());
  else if (key == "coins") g_options.startCoins = std::atoi(value.c_str());
  else if (key == "speed") g_options.timeScale = std::max(1, std::min(16, std::atoi(value.c_str())));
  else if (key == "god") g_options.god = Truthy(value);
  else if (key == "bot") g_options.bot = Truthy(value);
  else if (key == "log") g_options.log = Truthy(value);
  else if (key == "perf") g_options.perf = Truthy(value);
  else if (key == "bosshp") g_options.bossHp = std::atoi(value.c_str());
  else if (key == "bossphase") g_options.bossPhase = std::max(0, std::min(2, std::atoi(value.c_str())));
  else if (key == "immortal") g_options.immortal = Truthy(value);
  else if (key == "shop") g_options.startInShop = Truthy(value);
  else if (key == "heartdemo") g_options.heartDemo = Truthy(value);
  else if (key == "heightdemo") g_options.heightDemo = Truthy(value);
  else if (key == "stompdemo") g_options.stompDemo = value.empty() ? 1 : std::max(0, std::min(32, std::atoi(value.c_str())));
  else if (key == "hp") g_options.startHp = std::atoi(value.c_str());
  else if (key == "cards") g_options.cards = value;
  else if (key == "pick") g_options.startPicks = value.empty() ? 1 : std::atoi(value.c_str());
}

// "a=1&b=2" 또는 "--a=1" 한 토큰을 key/value로 나눈다.
void ApplyToken(std::string token) {
  while (!token.empty() && (token[0] == '-' || token[0] == '?')) token.erase(0, 1);
  if (token.empty()) return;
  const size_t eq = token.find('=');
  if (eq == std::string::npos) Apply(token, "");
  else Apply(token.substr(0, eq), token.substr(eq + 1));
}

}  // namespace

DebugOptions& Debug() { return g_options; }

void ParseDebugOptions(int argc, char** argv) {
#ifdef __EMSCRIPTEN__
  (void)argc;
  (void)argv;
  const char* query = emscripten_run_script_string("window.location.search");
  std::string q = query ? query : "";
  size_t start = 0;
  while (start <= q.size()) {
    const size_t amp = q.find('&', start);
    ApplyToken(q.substr(start, amp == std::string::npos ? std::string::npos : amp - start));
    if (amp == std::string::npos) break;
    start = amp + 1;
  }
#else
  for (int i = 1; i < argc; ++i) ApplyToken(argv[i]);
#endif
}

void DebugLog(const char* fmt, ...) {
  if (!g_options.log) return;
  va_list args;
  va_start(args, fmt);
  std::vprintf(fmt, args);
  va_end(args);
  std::printf("\n");
  std::fflush(stdout);
}

}  // namespace gengar
