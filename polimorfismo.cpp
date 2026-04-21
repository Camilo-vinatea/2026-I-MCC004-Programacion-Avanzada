#include <iostream>
#include <vector>
#include <memory>
#include <fstream>     // ofstream
#include "shapes/all-shapes.h"

using namespace std;

const int nObjs = 4;

void DemoPolimorfismo() {

    ofstream ofs("shapes.txt");
    ostream &os = cout;
    // Crear 3 objetos bajo punteros de tipo Shape
    Shape *sv[nObjs] = {new Rectangle("RectX", os, 4.0, 5.0), 
                        new Circle   ("CírcX", os, 3.0),
                        new Triangle ("TriaX", os, 6.0, 2.0),
                        new Square   ("SquaX", os, 4.0)
                       };
    cout << "Punto #1" << endl;
    for(int i = 0; i < nObjs; i++)
        cout << sv[i]->ToString() << endl;
    cout << "Punto #2" << endl;
    for(int i = 0; i < nObjs; i++)
        cout << "obj: " << i << " -> " << *sv[i] << " mensaje posterior" << endl;
    //. ===============
    //.            cout << i
    //             =========
    //                  cout << " -> "
    //                  ==============
    //                            cout << *sv[i]
    //                            ==============
    //                                      cout << " mensaje posterior"
    //                                      ============================
    //                                                              cout             
    for(int i = 0; i < nObjs; i++)
        delete sv[i];

    ofs << ">>>>>>>>>>>>>> Ahora viene la segunda parte" << endl;

    cout << "============== Demostración de polimorfismo ==============" << endl;
    // Crear un vector de punteros a Shape
    vector< unique_ptr<Shape> > formas;

    // Agregar figuras con nombre y dimensiones
    formas.push_back(make_unique<Rectangle>("Mi Rectángulo", os, 4.0, 5.0));
    formas.push_back(make_unique<Circle>("Mi Círculo", os, 3.0));
    formas.push_back(make_unique<Triangle>("Mi Triángulo", os, 6.0, 2.0));
    formas.push_back(make_unique<Square>("Mi Cuadrado", os, 4.0));

    // Recorrer y mostrar nombre y área usando polimorfismo
    
    for (const auto& forma : formas) {
        // Se llama a GetName() de Shape y GetArea() virtual
        cout << forma->ToString() << endl;
    }

    int a, b;
    float c;
    string s;
    ifstream ifs("entradas.txt");
    ifs >> a >> b >> c >> s;
    cout << "a: " << a << " b: " << b 
         << " c: " << c << " s: " << s << endl;
    // cin >> a;
    // ostream (cout)
    // ofstream      : archivos
    // ostringstream : strings
}