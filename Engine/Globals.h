#pragma once

// Aliases
using uint = unsigned int;
using byte = signed char;
using ubyte = unsigned char;

using cstr = const char*;

// Constants
extern const float M_PI;

// Functions
std::ostream& operator<<(std::ostream& os, const glm::vec2 v);
std::ostream& operator<<(std::ostream& os, const glm::ivec2 v);
std::ostream& operator<<(std::ostream& os, const glm::vec3 v);
std::ostream& operator<<(std::ostream& os, const glm::vec4 v);

void print(cstr str, bool endl=true);
void print_blue(cstr str, bool endl=true);
void print_green(cstr str, bool endl=true);
void print_yellow(cstr str, bool endl=true);
void print_red(cstr str, bool endl=true);

