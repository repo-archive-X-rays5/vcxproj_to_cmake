#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <algorithm>
#include "pugixml.hpp"

struct Project {
    std::string name;
    std::string version;
    std::string type;
    std::string subsystem;
    std::vector<std::string> source;

    std::string CmakeFile() {
        std::stringstream cmake;
        cmake << "cmake_minimum_required(VERSION 3.20)\n";
        cmake << "project(" << name << ")\n";
        cmake << "set(" << GetVersion() << ")\n";
        cmake << GetTarget();

        return cmake.str();
    }

private:
    int StringToInt(const std::string& convert) {
        std::stringstream result;
        for (auto&& c : convert) {
            if (std::isdigit(c))
                result << c;
        }
        int converted;
        result >> converted;
        return converted;
    }

    std::string GetVersion() {
        if (version.find("stdcpp") != std::string::npos) {
            std::string standard_version = "CMAKE_CXX_STANDARD ";
            standard_version.append(std::to_string(StringToInt(version)));
            return standard_version;
        } else if (version.find("stdc") != std::string::npos) {
            std::string standard_version = "CMAKE_C_STANDARD ";
            standard_version.append(std::to_string(StringToInt(version)));
            return standard_version;
        } else {
            return "CMAKE_CXX_STANDARD 14"; // if version can't be found default to cpp14
        }
    }

    std::string GetTarget() {
        std::stringstream target;
        if (type == "StaticLibrary" || type == "DynamicLibrary") {
            target << "add_library(" << name;
            if (subsystem == "Windows")
                target << " WIN32";
            if (type == "StaticLibrary") {
                target << " STATIC";
            } else {
                target << " SHARED";
            }
            target << GetFiles() << ")\n";
        } else {
            target << "add_executable(" << name;
            if (subsystem == "Windows")
                target << " WIN32";
            target << " " << GetFiles() << ")\n";
        }
        return target.str();
    }

    std::string GetFiles() {
        std::stringstream files;
        for (auto&& file : source)
            files << file << " ";
        return files.str();
    }
};

Project GetProject(const std::string& file) {
    pugi::xml_document xml;
    pugi::xml_parse_result result = xml.load_file(file.c_str());
    if (!result)
        return {};

    Project project;
    for (auto&& child : xml.child("Project")) {
        if (std::string(child.name()) == "PropertyGroup" && std::string(child.attribute("Label").as_string()) == "Configuration") {
            project.type = child.child("ConfigurationType").text().as_string();
            continue;
        }
        if (std::string(child.name()) == "PropertyGroup" && std::string(child.attribute("Label").as_string()) == "Globals") {
            if (child.child("RootNamespace").text().as_string()) {
                project.name = child.child("RootNamespace").text().as_string();
            } else if (child.child("ProjectName").text().as_string()) {
                project.name = child.child("ProjectName").text().as_string();
            } else {
                project.name = "NULL";
            }
            continue;
        }
        if (std::string(child.name()) == "ItemDefinitionGroup" && !child.attribute("Condition").empty()) {
            project.version = child.child("ClCompile").child("LanguageStandard").text().as_string();
            project.subsystem = child.child("Link").child("SubSystem").text().as_string();
            continue;
        }
        if (std::string(child.name()) == "ItemGroup" && child.child("ClCompile")) {
            for (auto&& compile : child.children()) {
                std::string path = std::string(compile.attribute("Include").value());
                std::replace(path.begin(), path.end(), '\\', '/');
                project.source.emplace_back(path);
            }
            continue;
        }
        if (std::string(child.name()) == "ItemGroup" && child.child("ClInclude")) {
            for (auto&& compile : child.children()) {
                std::string path = std::string(compile.attribute("Include").value());
                std::replace(path.begin(), path.end(), '\\', '/');
                project.source.emplace_back(path);
            }
            continue;
        }
    }
    return project;
}

int main(int argc, char* argv[]) {
    std::string file;
    if (argc > 1) {
        file = argv[1];
    } else {
        std::cout << "file to convert: ";
        std::getline(std::cin, file);
    }

    auto project = GetProject(file);

    std::ofstream writer("CMakeLists.txt");
    if (!writer.is_open()) {
        std::cerr << "Failed to open CMakeLists.txt for writing";
        return EXIT_FAILURE;
    } else {
        writer << project.CmakeFile();
        writer.close();
    }
    return EXIT_SUCCESS;
}
