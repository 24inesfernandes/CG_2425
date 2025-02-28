#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "camera.h" // Include your existing Camera class
#include "tinyxml2.h"

// Structure to store window settings
struct Window {
    int width, height;
    
    Window() : width(800), height(600) {}
};

// Structure to store model information
struct Model {
    std::string filename;
    // You might want to add more fields later for textures, colors, etc.
};

// Structure to represent a group in the scene graph
struct Group {
    std::vector<Model> models;
    std::vector<Group> children;
    // In later phases, you'll add transforms here
};

// Main parser class
class SimpleParser {
public:
    // Parse the entire XML file - now using your Camera class
    static bool parseXMLFile(const std::string& filename, Window& window, Camera& camera, Group& rootGroup);
    
private:
    // Parse a group element and its children recursively
    static void parseGroup(tinyxml2::XMLElement* groupElement, Group& group);
};