//
// Created by Inês Fernandes on 05/02/2025.
// Programa principal que recebe argumentos e decide que modelo gerar.
//

#include <iostream>
#include "model.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Uso: generator [tipo] [parametros] [ficheiro]\n";
        return 1;
    }

    string type = argv[1];
    string filename = argv[argc-1];

    if (type == "plane") {
        float size = stof(argv[2]);
        int divisions = stoi(argv[3]);
        generatePlane(size, divisions, filename);
    }
    else {
        cerr << "Tipo de modelo inválido!\n";
    }

    return 0;
}
