//
// Created by centu on 5/05/2021.
//

#ifndef ENGINE_DRAWTRIANGLES_H
#define ENGINE_DRAWTRIANGLES_H
//#include "easy_image.h"
//#include "Hulpfuncties.h"
//#include "ZBuffer.h"
//#include <list>
#include "L3Dsysteem.h"
#include "Lsysteem.h"

using namespace std;

class L3Dsysteem;

class drawTriangles {

};

img::EasyImage drawTriangles(const int size, const vector<double>& backgroundColor, list<Figure> &figuren, Lights3D &lichten, bool lighted, vector<double> eye);

void draw_zbuf_triag(img::EasyImage &image,
                     ZBuffer &zbuffer,
                     Vector3D const &A,
                     Vector3D const &B,
                     Vector3D const &C,
                     double d, double dx, double dy,
                     Color &ambientReflection, Color &diffuseReflection, Color specularReflection, double reflectionCoeff, Lights3D &lights, bool lighted, vector<double> eye);


#endif //ENGINE_DRAWTRIANGLES_H
