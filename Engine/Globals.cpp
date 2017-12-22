#include "stdafx.h"
#include "Globals.h"
#include "ConsoleColors.h"

const float M_PI = 3.14159265358979323846f;

std::string RESOURCES_FOLDER = "../Resources/";

std::ostream & operator<<(std::ostream & os, const glm::vec2 v) {
	return os << "{ " << v.x << ", " << v.y << " }";
}
std::ostream & operator<<(std::ostream & os, const glm::ivec2 v) {
	return os << "{ " << v.x << ", " << v.y << " }";
}
std::ostream & operator<<(std::ostream & os, const glm::vec3 v) {
	return os << "{ " << v.x << ", " << v.y << ", " << v.z << " }";
}
std::ostream & operator<<(std::ostream & os, const glm::vec4 v) {
	return os << "{ " << v.x << ", " << v.y << ", " << v.z << ", " << v.w << " }";
}

void print(cstr str, bool endl) {
	std::cout << white << str;
	if (endl) std::cout << '\n';
}
void print_blue(cstr str, bool endl) {
	std::cout << blue << str << white;
	if (endl) std::cout << '\n';
}
void print_green(cstr str, bool endl) {
	std::cout << green << str << white;
	if (endl) std::cout << '\n';
}
void print_yellow(cstr str, bool endl) {
	std::cout << yellow << str << white;
	if (endl) std::cout << '\n';
}
void print_red(cstr str, bool endl) {
	std::cout << red << str << white;
	if (endl) std::cout << '\n';
}
