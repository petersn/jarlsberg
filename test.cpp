 // Test case for surfaces
using namespace std;
#include <iostream>
#include <list>
#include "surfaces.h"
#include "surfaces.cpp"

int main() {

    std::cout << "Jarlsberg Test\n";

    vector<double> A = vector<double>(1.7, 8.9, -0.8);
    vector<double> B = vector<double>(0.4, 5.4, 3.2);

    std::cout << "A = " << A << endl;
    std::cout << "B = " << B << endl;

    std::cout << "A + B = " << (A + B) << endl;
    std::cout << "A - B = " << (A - B) << endl;
    std::cout << "A * B = " << (A * B) << endl;
    std::cout << "A x B = " << (A ^ B) << endl;

    matrix<double> proj = matrix<double>(vector<double>(1.0, 0.0, 0.0), vector<double>(0.0, 1.0, 0.0), vector<double>(0.0, 0.0, 1.0));

    std::cout << proj;

    surface s = surface(vector<int>(0,0,0), 5, 5, vector<int>(0,0,-1), vector<int>(1,0,0) );

    std::cout << s;

}

