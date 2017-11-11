#include "stdafx.h"
#include "ObjParser.h"
#include "ConsoleColors.h"

ObjParser::ObjParser(cstr filename, cstr root_folder)
{
	ParseFile(filename, root_folder);
}

void ObjParser::ParseFile(cstr filename, cstr root_folder)
{
	data.clear();
	vertices_data.clear();

	std::ifstream ss((std::string(root_folder) + filename).c_str());
	if (!ss.is_open()) {
		std::cout << red << "ERROR: ObjParser::ParseFile() failed to open file." << yellow <<
			std::string(root_folder) + filename << white << '\n';
		return;
	}

	std::string w;
	float f1, f2, f3;
	int i;
	std::vector<glm::vec3> verts, norms;
	while (ss >> w) {
		if (w == "v") {
			ss >> f1;
			ss >> f2;
			ss >> f3;
			verts.push_back({ f1, f2, f3 });
		}
		else if (w == "vn") {
			ss >> f1;
			ss >> f2;
			ss >> f3;
			norms.push_back({ f1,f2,f3 });
		}
		else if (w == "s") { // smooth shading. We only handle 'off' for now.
			ss >> w;
			if (w != "off") {
				std::cout << red << "File " << yellow << std::string(root_folder) + filename
					<< red << " can't be parsed. (smooth shading must be off)" << white << '\n';
				return;
			}
		}
		else if (w == "f") {
			for (int j = 0; j != 3; ++j) {
				ss >> w;
				i = std::stoi(w.substr(0, w.find('/')));
				data.push_back(verts[i - 1]);
				vertices_data.push_back(verts[i - 1]);
				i = std::stoi(w.substr(w.find_last_of('/') + 1, w.size()));
				data.push_back(norms[i - 1]);
			}
		}
	}

}
