//
// Created by Inês Fernandes on 05/02/2025.
//

#include "model.h"
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

// Cria um ficheiro .3d com os vértices do plano
void generatePlane(float size, int divisions, const string& filename) {
    ofstream file(filename);
    if (!file) {
        cerr << "Erro ao criar o ficheiro!" << endl;
        return;
    }

    vector<float> vertices;
    float step = size / divisions;
    float half = size / 2.0f;

    for (int i = 0; i < divisions; i++) {
        for (int j = 0; j < divisions; j++) {
            float x1 = -half + j * step;
            float z1 = -half + i * step;
            float x2 = x1 + step;
            float z2 = z1 + step;

            vertices.insert(vertices.end(), {x1, 0, z1, x2, 0, z1, x1, 0, z2});
            vertices.insert(vertices.end(), {x2, 0, z1, x2, 0, z2, x1, 0, z2});
        }
    }

    file << vertices.size() / 3 << endl;
    for (size_t i = 0; i < vertices.size(); i += 3) {
        file << vertices[i] << " " << vertices[i+1] << " " << vertices[i+2] << endl;
    }

    file.close();
    cout << "Ficheiro " << filename << " criado!" << endl;
}
