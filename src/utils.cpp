#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

#include "utils.hpp"

// using namespace std;

namespace utils {

bool exists_file(std::string filename) {
  return std::filesystem::exists(filename);
}

std::string read_file(std::string filename) {
  std::ifstream inf;
  inf.open(filename);
  std::stringstream buffer;
  buffer << inf.rdbuf();
  //  std::cout << buffer.str();
  inf.close();
  return buffer.str();
}

std::vector<std::string> read_lines(std::string filename) {
  std::vector<std::string> res;
  std::ifstream inputFile(filename);
  std::string line;
  while (std::getline(inputFile, line)) {
    res.push_back(line);
  }
  return res;
}

/* use boost algo...
bool starts_with(std::string source, std::string prefix)
{
  return source.rfind(prefix, 0);
}
*/

std::string get_uuid() {
  static std::random_device dev;
  static std::mt19937 rng(dev());

  std::uniform_int_distribution<int> dist(0, 15);

  const char *v = "0123456789abcdef";
  const bool dash[] = {0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0};

  std::string res;
  for (int i = 0; i < 16; i++) {
    if (dash[i])
      res += "-";
    res += v[dist(rng)];
    res += v[dist(rng)];
  }
  return res;
}

bool url_decode(const std::string &in, std::string &out) {
  out.clear();
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i) {
    if (in[i] == '%') {
      if (i + 3 <= in.size()) {
        int value = 0;
        std::istringstream is(in.substr(i + 1, 2));
        if (is >> std::hex >> value) {
          out += static_cast<char>(value);
          i += 2;
        } else {
          return false;
        }
      } else {
        return false;
      }
    } else if (in[i] == '+') {
      out += ' ';
    } else {
      out += in[i];
    }
  }
  return true;
}

std::map<std::string, std::string>
double_split(const std::string &in, const char primary, const char secondary) {
  std::map<std::string, std::string> rm;

  std::vector<std::string> parts;
  boost::split(parts, in, [primary](char c) { return c == primary; });
  for (auto part : parts) {

    std::vector<std::string> innerparts;
    boost::split(innerparts, part,
                 [secondary](char c) { return c == secondary; });

    if (innerparts.size() > 1) {
      auto a = innerparts[0];
      //      auto b = boost::join(
      //          std::vector<std::string>(innerparts.begin() + 1,
      //          innerparts.end()),
      //          "&");
      auto b = innerparts[1];
      std::string c;
      url_decode(b, c);
      rm.insert(std::pair(a, c));
    }
  }
  return rm;
}

std::map<std::string, std::string> url_to_map(const std::string &in) {
  return double_split(in, '&', '=');
}

void url_to_map(std::map<std::string, std::string> *mp, const std::string &in) {
  auto x = double_split(in, '&', '=');
  mp->merge(x);
}

int s2i(std::string s, int d) {
  try {
    auto r = std::stoi(s);
    return r;
  } catch (...) {
    return d;
  }
}

std::string i2s(int i, std::string d) { return std::to_string(i); }

double s2d(std::string s, double d) {
  try {
    auto r = std::stod(s);
    return r;
  } catch (...) {
    return d;
  }
}

std::string d2s(double i, std::string d) { return std::to_string(i); }

long s2l(std::string s, long d) {
  try {
    auto r = std::stol(s);
    return r;
  } catch (...) {
    return d;
  }
}

std::string l2s(long i, std::string d) { return std::to_string(i); }

std::string join(std::vector<std::string> vec, std::string sep) {
  std::stringstream joinedValues;
  for (auto value : vec) {
    joinedValues << value << ",";
  }
  std::string result = joinedValues.str();
  result.pop_back();
  return result;
}

template <class T, class A> T join(const A &begin, const A &end, const T &t) {
  T result;
  for (A it = begin; it != end; it++) {
    if (!result.empty())
      result.append(t);
    result.append(*it);
  }
  return result;
}

long timestamp() {
  auto time = std::chrono::system_clock::now().time_since_epoch();
  auto secs = std::chrono::duration_cast<std::chrono::seconds>(time).count();
  return secs;
}

} // namespace utils
