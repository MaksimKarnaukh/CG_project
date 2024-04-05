//
// Created by centu on 17/05/2021.
//

#ifndef ENGINE_LIGHT_H
#define ENGINE_LIGHT_H
#include "Lsysteem.h"
#include "vector3d.h"

class Light {
public:
    Light();

    Color ambientLight;
    Color diffuseLight;
    Color specularLight;

    bool infinity;
    bool isSpotAngle;
    bool isSpecular;
    // dit was een tip van een medeleerling om deze datamembers ook in deze klasse te includen, wat alles makkelijker maakt.
    Vector3D ldVector; // de richting waarin het licht schijnt

    Vector3D location; // de locatie van de puntbron
    double spotAngle; // de hoek van een spotlight

    bool isShadow;
    ZBuffer shadowMask1;
    Matrix eye;
    double d, dx, dy;

};

typedef std::list<Light> Lights3D;

#endif //ENGINE_LIGHT_H
