#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>
#include <GL/glut.h>
#include "tinyxml2.h"
#include "camera.h"
#include <map>

using namespace std;
using namespace tinyxml2;

// Structure to represent a 3D vertex
struct Vertex {
    float x, y, z;
    
    Vertex() : x(0), y(0), z(0) {}
    Vertex(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};

// Structure to represent a face (triangle)
struct Face {
    int v1, v2, v3;  // Vertex indices
    
    Face() : v1(0), v2(0), v3(0) {}
    Face(int _v1, int _v2, int _v3) : v1(_v1), v2(_v2), v3(_v3) {}
};

// Structure to represent a 3D model
struct Model {
    string filename;
    vector<Vertex> vertices;
    vector<Face> faces;
    
    bool loaded;
    
    Model() : loaded(false) {}
};

// Structure for Window settings
struct Window {
    int width, height;
    
    Window() : width(800), height(600) {}
};

// Structure for a group node (for Phase 2)
struct Group {
    // Will contain transforms in Phase 2
    vector<Model> models;
    vector<Group> childGroups;
};

// Global variables
Window window;
Camera* camera;
Group rootGroup;

bool showAxes = false;
bool wireframeMode = false;

// Function prototypes
bool parseXMLFile(const string& filename);
bool loadModel(Model& model);
void parseGroup(XMLElement* groupElement, Group& group);
void renderGroup(const Group& group);
void changeSize(int w, int h);
void renderScene();
void drawAxes();
void processKeys(unsigned char key, int xx, int yy);
void processSpecialKeys(int key, int xx, int yy);

int main(int argc, char** argv) {
    // Check if config file is provided
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <config.xml>" << endl;
        return 1;
    }
    
    // Create camera with default values
    camera = new Camera();
    
    // Parse the XML file
    if (!parseXMLFile(argv[1])) {
        cerr << "Failed to parse XML file." << endl;
        return 1;
    }
    
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(window.width, window.height);
    glutCreateWindow("3D Engine");
    
    // Register callback functions
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);
    glutKeyboardFunc(processKeys);
    glutSpecialFunc(processSpecialKeys);
    
    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    // Display keyboard controls
    cout << "\n--- 3D Engine Controls ---" << endl;
    cout << "Arrow keys: Rotate camera" << endl;
    cout << "W/S: Zoom in/out" << endl;
    cout << "A: Toggle axes display" << endl;
    cout << "L: Toggle wireframe mode" << endl;
    
    // Enter GLUT main loop
    glutMainLoop();
    
    // Clean up
    delete camera;
    
    return 0;
}

// Parse the XML file using TinyXML2
bool parseXMLFile(const string& filename) {
    XMLDocument doc;
    if (doc.LoadFile(filename.c_str()) != XML_SUCCESS) {
        cerr << "Error loading XML file: " << filename << endl;
        return false;
    }
    
    // Get the root element (world)
    XMLElement* worldElement = doc.FirstChildElement("world");
    if (!worldElement) {
        cerr << "Missing 'world' element in XML file." << endl;
        return false;
    }
    
    // Parse window settings
    XMLElement* windowElement = worldElement->FirstChildElement("window");
    if (windowElement) {
        windowElement->QueryIntAttribute("width", &window.width);
        windowElement->QueryIntAttribute("height", &window.height);
    }
    
    // Parse camera settings
    XMLElement* cameraElement = worldElement->FirstChildElement("camera");
    if (cameraElement) {
        float posX = 0, posY = 0, posZ = 5;
        float lookX = 0, lookY = 0, lookZ = 0;
        float upX = 0, upY = 1, upZ = 0;
        float fov = 60, near = 1, far = 1000;
        
        XMLElement* posElement = cameraElement->FirstChildElement("position");
        if (posElement) {
            posElement->QueryFloatAttribute("x", &posX);
            posElement->QueryFloatAttribute("y", &posY);
            posElement->QueryFloatAttribute("z", &posZ);
        }
        
        XMLElement* lookAtElement = cameraElement->FirstChildElement("lookAt");
        if (lookAtElement) {
            lookAtElement->QueryFloatAttribute("x", &lookX);
            lookAtElement->QueryFloatAttribute("y", &lookY);
            lookAtElement->QueryFloatAttribute("z", &lookZ);
        }
        
        XMLElement* upElement = cameraElement->FirstChildElement("up");
        if (upElement) {
            upElement->QueryFloatAttribute("x", &upX);
            upElement->QueryFloatAttribute("y", &upY);
            upElement->QueryFloatAttribute("z", &upZ);
        }
        
        XMLElement* projElement = cameraElement->FirstChildElement("projection");
        if (projElement) {
            projElement->QueryFloatAttribute("fov", &fov);
            projElement->QueryFloatAttribute("near", &near);
            projElement->QueryFloatAttribute("far", &far);
        }
        
        // Update camera with parsed values
        camera->setPosition(posX, posY, posZ);
        camera->setLookAt(lookX, lookY, lookZ);
        camera->setUp(upX, upY, upZ);
        camera->setProjection(fov, near, far);
    }
    
    // Parse groups (recursively)
    XMLElement* groupElement = worldElement->FirstChildElement("group");
    if (groupElement) {
        parseGroup(groupElement, rootGroup);
    }
    
    return true;
}

// Recursively parse group elements
void parseGroup(XMLElement* groupElement, Group& group) {
    // Parse models in this group
    XMLElement* modelsElement = groupElement->FirstChildElement("models");
    if (modelsElement) {
        XMLElement* modelElement = modelsElement->FirstChildElement("model");
        while (modelElement) {
            const char* filename = modelElement->Attribute("file");
            if (filename) {
                Model model;
                model.filename = filename;
                
                // Load the model
                if (loadModel(model)) {
                    group.models.push_back(model);
                }
            }
            
            modelElement = modelElement->NextSiblingElement("model");
        }
    }
    
    // Parse child groups (for Phase 2)
    XMLElement* childGroupElement = groupElement->FirstChildElement("group");
    while (childGroupElement) {
        Group childGroup;
        parseGroup(childGroupElement, childGroup);
        group.childGroups.push_back(childGroup);
        
        childGroupElement = childGroupElement->NextSiblingElement("group");
    }
}

// Load a 3D model from file (XML format)
bool loadModel(Model& model) {
    ifstream file(model.filename);
    if (!file.is_open()) {
        cerr << "Error opening model file: " << model.filename << endl;
        return false;
    }
    
    // Clear any existing data
    model.vertices.clear();
    model.faces.clear();
    
    // Read the entire file content into a string
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    
    // Parse XML content using TinyXML2
    XMLDocument doc;
    if (doc.Parse(content.c_str()) != XML_SUCCESS) {
        cerr << "Error parsing XML in model file: " << model.filename << endl;
        return false;
    }
    
    // Get the root element (should be one of: plane, box, sphere, cone)
    XMLElement* rootElement = doc.RootElement();
    if (!rootElement) {
        cerr << "No root element found in model file: " << model.filename << endl;
        return false;
    }
    
    // Maps to store vertex indices
    map<string, int> vertexIndices;
    int nextIndex = 0;
    
    // Process all triangle elements
    XMLElement* triangleElement = rootElement->FirstChildElement("triangle");
    int faceCount = 0;
    
    while (triangleElement) {
        XMLElement* vertex1 = triangleElement->FirstChildElement("vertex");
        XMLElement* vertex2 = vertex1 ? vertex1->NextSiblingElement("vertex") : nullptr;
        XMLElement* vertex3 = vertex2 ? vertex2->NextSiblingElement("vertex") : nullptr;
        
        if (vertex1 && vertex2 && vertex3) {
            // Create three vertices for the triangle
            vector<int> vertexIndicesForTriangle;
            
            // Process each vertex of the triangle
            for (XMLElement* vertex : {vertex1, vertex2, vertex3}) {
                float x = 0, y = 0, z = 0;
                vertex->QueryFloatAttribute("x", &x);
                vertex->QueryFloatAttribute("y", &y);
                vertex->QueryFloatAttribute("z", &z);
                
                // Create a unique key for this vertex
                string vertexKey = to_string(x) + "," + to_string(y) + "," + to_string(z);
                
                // Check if we've seen this vertex before
                if (vertexIndices.find(vertexKey) == vertexIndices.end()) {
                    // New vertex, add it to the model
                    model.vertices.push_back(Vertex(x, y, z));
                    vertexIndices[vertexKey] = nextIndex;
                    vertexIndicesForTriangle.push_back(nextIndex);
                    nextIndex++;
                } else {
                    // Existing vertex, reuse its index
                    vertexIndicesForTriangle.push_back(vertexIndices[vertexKey]);
                }
            }
            
            // Add the face if we have three valid vertices
            if (vertexIndicesForTriangle.size() == 3) {
                model.faces.push_back(Face(
                    vertexIndicesForTriangle[0],
                    vertexIndicesForTriangle[1],
                    vertexIndicesForTriangle[2]
                ));
                faceCount++;
            }
        } else {
            cerr << "Triangle missing vertices in model file: " << model.filename << endl;
        }
        
        triangleElement = triangleElement->NextSiblingElement("triangle");
    }
    
    model.loaded = true;
    cout << "Model loaded: " << model.filename << " (" << model.vertices.size() << " vertices, " 
         << faceCount << " faces)" << endl;
    
    return true;
}

// Recursively render a group and its children
void renderGroup(const Group& group) {
    // In Phase 2, apply transforms here
    
    // Render all models in this group
    for (const Model& model : group.models) {
        if (!model.loaded) continue;
        
        // If model has faces defined, use them for rendering
        if (!model.faces.empty()) {
            glBegin(GL_TRIANGLES);
            for (const Face& face : model.faces) {
                // Use alternating colors for triangles
                static int colorToggle = 0;
                if (colorToggle % 2 == 0) {
                    glColor3f(0.8f, 0.6f, 0.2f);  // Orange-ish
                } else {
                    glColor3f(0.2f, 0.6f, 0.8f);  // Blue-ish
                }
                colorToggle++;
                
                // Draw the triangle
                const Vertex& v1 = model.vertices[face.v1];
                const Vertex& v2 = model.vertices[face.v2];
                const Vertex& v3 = model.vertices[face.v3];
                
                glVertex3f(v1.x, v1.y, v1.z);
                glVertex3f(v2.x, v2.y, v2.z);
                glVertex3f(v3.x, v3.y, v3.z);
            }
            glEnd();
        } else {
            // No faces defined, render vertices directly in triangle order
            glBegin(GL_TRIANGLES);
            for (size_t i = 0; i < model.vertices.size(); i += 3) {
                if (i + 2 < model.vertices.size()) {
                    // Use alternating colors for triangles
                    static int colorToggle = 0;
                    if (colorToggle % 2 == 0) {
                        glColor3f(0.8f, 0.6f, 0.2f);  // Orange-ish
                    } else {
                        glColor3f(0.2f, 0.6f, 0.8f);  // Blue-ish
                    }
                    colorToggle++;
                    
                    // Draw the triangle
                    const Vertex& v1 = model.vertices[i];
                    const Vertex& v2 = model.vertices[i + 1];
                    const Vertex& v3 = model.vertices[i + 2];
                    
                    glVertex3f(v1.x, v1.y, v1.z);
                    glVertex3f(v2.x, v2.y, v2.z);
                    glVertex3f(v3.x, v3.y, v3.z);
                }
            }
            glEnd();
        }
    }
    
    // Recursively render child groups
    for (const Group& childGroup : group.childGroups) {
        glPushMatrix();
        renderGroup(childGroup);
        glPopMatrix();
    }
}

// GLUT reshape function
void changeSize(int w, int h) {
    // Prevent division by zero
    if (h == 0) h = 1;
    
    // Compute window's aspect ratio
    float ratio = w * 1.0f / h;
    
    // Set the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Set the viewport
    glViewport(0, 0, w, h);
    
    // Set perspective
    gluPerspective(camera->getFov(), ratio, camera->getNearPlane(), camera->getFarPlane());
    
    // Return to modelview matrix
    glMatrixMode(GL_MODELVIEW);
}

// Draw coordinate axes
void drawAxes() {
    glBegin(GL_LINES);
    
    // X axis (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-100.0f, 0.0f, 0.0f);
    glVertex3f(100.0f, 0.0f, 0.0f);
    
    // Y axis (green)
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, -100.0f, 0.0f);
    glVertex3f(0.0f, 100.0f, 0.0f);
    
    // Z axis (blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, -100.0f);
    glVertex3f(0.0f, 0.0f, 100.0f);
    
    glEnd();
}

// GLUT display function
void renderScene() {
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set polygon drawing mode
    if (wireframeMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    // Set the camera
    glLoadIdentity();
    camera->place();
    
    // Draw axes if enabled
    if (showAxes) {
        drawAxes();
    }
    
    // Render the scene (starting from root group)
    renderGroup(rootGroup);
    
    // Swap buffers
    glutSwapBuffers();
}

// Keyboard input processing
void processKeys(unsigned char key, int xx, int yy) {
    switch (key) {
        case 'a':
        case 'A':
            showAxes = !showAxes;
            break;
        
        case 'l':
        case 'L':
            wireframeMode = !wireframeMode;
            break;
        
        case 'w':
        case 'W':
            camera->zoomIn();
            break;
        
        case 's':
        case 'S':
            camera->zoomOut();
            break;
        
        case 27:  // Escape key
            exit(0);
            break;
    }
    
    glutPostRedisplay();
}

// Special key input processing
void processSpecialKeys(int key, int xx, int yy) {
    switch (key) {
        case GLUT_KEY_UP:
            camera->rotateUp();
            break;
        
        case GLUT_KEY_DOWN:
            camera->rotateDown();
            break;
        
        case GLUT_KEY_LEFT:
            camera->rotateLeft();
            break;
        
        case GLUT_KEY_RIGHT:
            camera->rotateRight();
            break;
    }
    
    glutPostRedisplay();
}