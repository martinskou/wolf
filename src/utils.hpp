#pragma once

#include <boost/algorithm/string.hpp>

// using namespace std;

namespace utils {

bool exists_file(std::string filename);

std::string read_file(std::string filename);

std::vector<std::string> read_lines(std::string filename);

std::string get_uuid();

bool url_decode(const std::string &in, std::string &out);

std::map<std::string, std::string> url_to_map(const std::string &in);
void url_to_map(std::map<std::string, std::string> *mp, const std::string &in);

int s2i(std::string, int = 0);
std::string i2s(int, std::string = "");

double s2d(std::string, double = 0);
std::string d2s(double, std::string = "");

long s2l(std::string, long = 0);
std::string l2s(long, std::string = "");

std::string join(std::vector<std::string>, std::string);

long timestamp();

/* use boost algo...
bool starts_with(std::string source, std::string prefix)
{
  return source.rfind(prefix, 0);
}
*/

} // namespace utils

namespace color {
enum Code {
  FG_RED = 31,
  FG_GREEN = 32,
  FG_BLUE = 34,
  FG_DEFAULT = 39,
  BG_RED = 41,
  BG_GREEN = 42,
  BG_BLUE = 44,
  BG_DEFAULT = 49
};
class Modifier {
  Code code;

public:
  Modifier(Code pCode) : code(pCode) {}
  friend std::ostream &operator<<(std::ostream &os, const Modifier &mod) {
    return os << "\033[" << mod.code << "m";
  }
};
} // namespace color
