//
// Created by centu on 5/03/2021.
//

# define M_PI   3.14159265358979323846 // gevonden op https://stackoverflow.com/questions/1727881/how-to-use-the-pi-constant-in-c

#include "L3Dsysteem.h"

Matrix L3Dsysteem::scaleFigure(const double scale) {
    Matrix s;
    s(1, 1) = scale;
    s(2, 2) = scale;
    s(3, 3) = scale;

    return s;
}

Matrix L3Dsysteem::rotateX(const double angle) {

    Matrix Mx;
    double angleRad = angle*(M_PI/180);
    Mx(2, 2) = cos(angleRad);
    Mx(2, 3) = sin(angleRad);
    Mx(3, 2) = -sin(angleRad);
    Mx(3, 3) = cos(angleRad);

    return Mx;
}

Matrix L3Dsysteem::rotateY(const double angle) {
    Matrix My;
    double angleRad = angle*(M_PI/180);
    My(1, 1) = cos(angleRad);
    My(1, 3) = -sin(angleRad);
    My(3, 1) = sin(angleRad);
    My(3, 3) = cos(angleRad);

    return My;
}

Matrix L3Dsysteem::rotateZ(const double angle) {
    Matrix Mz;
    double angleRad = angle*(M_PI/180);
    Mz(1, 1) = cos(angleRad);
    Mz(1, 2) = sin(angleRad);
    Mz(2, 1) = -sin(angleRad);
    Mz(2, 2) = cos(angleRad);

    return Mz;
}

Matrix L3Dsysteem::translate(const Vector3D &vector) {
    Matrix T;
    T(4, 1) = vector.x;
    T(4, 2) = vector.y;
    T(4, 3) = vector.z;

    return T;
}

list<Line2D> L3Dsysteem::parse(const ini::Configuration &configuration, int nrFigures, const vector<double>& eye, bool zbufTriag, bool lighted, int nrLights) {
    Figures3D figuren;
    Lights3D lichten;

    bool isShadowEnabled = false;
    int shadowMask = 0;
    if (lighted) {
        if (configuration["General"]["shadowEnabled"].exists()) {
            isShadowEnabled = configuration["General"]["shadowEnabled"].as_bool_or_die();
            shadowMask = configuration["General"]["shadowMask"].as_int_or_die();
        }
        for (auto i = 0; i < nrLights; i++) {
            string lightNaam = "Light" + to_string(i);
            Light licht;

            licht.isShadow = isShadowEnabled;
            if (configuration[lightNaam]["ambientLight"].exists()) {
                vector<double> ambientReflection = configuration[lightNaam]["ambientLight"].as_double_tuple_or_die();
                licht.ambientLight.red = ambientReflection[0];
                licht.ambientLight.green = ambientReflection[1];
                licht.ambientLight.blue = ambientReflection[2];
            }
            else {
                licht.ambientLight.red = 0;
                licht.ambientLight.green = 0;
                licht.ambientLight.blue = 0;
            }
            if (configuration[lightNaam]["diffuseLight"].exists()) {
                vector<double> diffuseReflection = configuration[lightNaam]["diffuseLight"].as_double_tuple_or_die();
                licht.diffuseLight.red = diffuseReflection[0];
                licht.diffuseLight.green = diffuseReflection[1];
                licht.diffuseLight.blue = diffuseReflection[2];
            }
            else {
                licht.diffuseLight.red = 0;
                licht.diffuseLight.green = 0;
                licht.diffuseLight.blue = 0;
            }
            if (configuration[lightNaam]["specularLight"].exists()) {
                vector<double> specularReflection = configuration[lightNaam]["specularLight"].as_double_tuple_or_die();
                licht.specularLight.red = specularReflection[0];
                licht.specularLight.green = specularReflection[1];
                licht.specularLight.blue = specularReflection[2];
                licht.isSpecular = true;
            }
            else {
                licht.specularLight.red = 0;
                licht.specularLight.green = 0;
                licht.specularLight.blue = 0;
                licht.isSpecular = false;
            }

            if (configuration[lightNaam]["infinity"].exists()) {
                bool infinity = configuration[lightNaam]["infinity"].as_bool_or_die();
                if (infinity) {
                    licht.infinity = true;
                    if (configuration[lightNaam]["direction"].exists()) {
                        vector<double> direction = configuration[lightNaam]["direction"].as_double_tuple_or_die();
                        licht.ldVector.x = direction[0];
                        licht.ldVector.y = direction[1];
                        licht.ldVector.z = direction[2];
                    }
                }
                else {
                    licht.infinity = false;
                    if (configuration[lightNaam]["location"].exists()) {
                        vector<double> location = configuration[lightNaam]["location"].as_double_tuple_or_die();
                        licht.location.x = location[0];
                        licht.location.y = location[1];
                        licht.location.z = location[2];
                    }
                    if (configuration[lightNaam]["spotAngle"].exists()) {
                        double spotAngle = configuration[lightNaam]["spotAngle"].as_double_or_die();
                        licht.spotAngle = spotAngle;
                        licht.isSpotAngle = true;
                    }
                    else {
                        licht.isSpotAngle = false;
                    }
                }
            }
            lichten.push_back(licht);
        }
    }
    else { // niet belicht
        Light licht;
        licht.ambientLight.red = 1.0;
        licht.ambientLight.green = 1.0;
        licht.ambientLight.blue = 1.0;

        licht.diffuseLight.red = 0.0;
        licht.diffuseLight.green = 0.0;
        licht.diffuseLight.blue = 0.0;

        licht.specularLight.red = 0.0;
        licht.specularLight.green = 0.0;
        licht.specularLight.blue = 0.0;

        lichten.push_back(licht);
    }
    lights = lichten;
    for (auto i = 0; i < nrFigures; i++) {
        string figuurNaam = "Figure" + to_string(i);
        Figure figuur;

        double rotateX = configuration[figuurNaam]["rotateX"].as_double_or_die();
        double rotateY = configuration[figuurNaam]["rotateY"].as_double_or_die();
        double rotateZ = configuration[figuurNaam]["rotateZ"].as_double_or_die();

        double scale = configuration[figuurNaam]["scale"].as_double_or_die();

        vector<double> center = configuration[figuurNaam]["center"].as_double_tuple_or_die();

        Vector3D centerPunt;
        centerPunt.x = center[0];
        centerPunt.y = center[1];
        centerPunt.z = center[2];

        vector<double> ambient;
        vector<double> diffuse;
        vector<double> specular;
        vector<double> color;
        double reflectionCoefficient = 0;

        if (!lighted) {
            color = configuration[figuurNaam]["color"].as_double_tuple_or_die();
            figuur.ambientReflection.red = color[0];
            figuur.ambientReflection.green = color[1];
            figuur.ambientReflection.blue = color[2];

        }
        else {
            if (configuration[figuurNaam]["ambientReflection"].exists()) {
                ambient = configuration[figuurNaam]["ambientReflection"].as_double_tuple_or_die();
                figuur.ambientReflection.red = ambient[0];
                figuur.ambientReflection.blue = ambient[2];
                figuur.ambientReflection.green = ambient[1];
            }
            else {
                ambient.push_back(0);
                ambient.push_back(0);
                ambient.push_back(0);

            }
            if (configuration[figuurNaam]["diffuseReflection"].exists()) {
                diffuse = configuration[figuurNaam]["diffuseReflection"].as_double_tuple_or_die();
                figuur.diffuseReflection.red = diffuse[0];
                figuur.diffuseReflection.blue = diffuse[2];
                figuur.diffuseReflection.green = diffuse[1];
            }
            else {
                diffuse.push_back(0);
                diffuse.push_back(0);
                diffuse.push_back(0);

            }
            if (configuration[figuurNaam]["specularReflection"].exists()) {
                specular = configuration[figuurNaam]["specularReflection"].as_double_tuple_or_die();
                figuur.specularReflection.red = specular[0];
                figuur.specularReflection.blue = specular[2];
                figuur.specularReflection.green = specular[1];
            }
            else {
                specular.push_back(0);
                specular.push_back(0);
                specular.push_back(0);
            }
            if (configuration[figuurNaam]["reflectionCoefficient"].exists()) {
                reflectionCoefficient = configuration[figuurNaam]["reflectionCoefficient"].as_double_or_die();
                figuur.reflectionCoefficient = reflectionCoefficient;
            }
            else {
                figuur.reflectionCoefficient = 0;
            }

        }

        if (configuration[figuurNaam]["type"].as_string_or_die() == "Cube" || configuration[figuurNaam]["type"].as_string_or_die() == "FractalCube") {

            Figure kubus = createCube();

            if (!lighted) {
                kubus.ambientReflection.red = color[0];
                kubus.ambientReflection.blue = color[2];
                kubus.ambientReflection.green = color[1];
            }
            else {
                kubus.ambientReflection.red = ambient[0];
                kubus.ambientReflection.blue = ambient[2];
                kubus.ambientReflection.green = ambient[1];

                kubus.diffuseReflection.red = diffuse[0];
                kubus.diffuseReflection.green = diffuse[1];
                kubus.diffuseReflection.blue = diffuse[2];

                kubus.specularReflection.red = specular[0];
                kubus.specularReflection.green = specular[1];
                kubus.specularReflection.blue = specular[2];

                kubus.reflectionCoefficient = reflectionCoefficient;
            }

            applyTranformation(kubus, rotateX, rotateY, rotateZ, scale, centerPunt, eye);

            if (configuration[figuurNaam]["type"].as_string_or_die() == "FractalCube") {
                Figures3D fractaalFiguren;
                fractaalFiguren.push_back(kubus);
                double fractalScale = configuration[figuurNaam]["fractalScale"].as_double_or_die();
                int nr_iterations1 = configuration[figuurNaam]["nrIterations"].as_int_or_die();
                generateFractal(kubus, fractaalFiguren, nr_iterations1, fractalScale);
                for (auto it = fractaalFiguren.begin(); it != fractaalFiguren.end(); it++) {
                    figuren.push_back(*it);
                }
            }
            else {
                figuren.push_back(kubus);
            }
        }
        else if (configuration[figuurNaam]["type"].as_string_or_die() == "MengerSponge") {
            Figure kubus = createCube();

            if (!lighted) {
                kubus.ambientReflection.red = color[0];
                kubus.ambientReflection.blue = color[2];
                kubus.ambientReflection.green = color[1];
            }
            else {
                kubus.ambientReflection.red = ambient[0];
                kubus.ambientReflection.blue = ambient[2];
                kubus.ambientReflection.green = ambient[1];

                kubus.diffuseReflection.red = diffuse[0];
                kubus.diffuseReflection.green = diffuse[1];
                kubus.diffuseReflection.blue = diffuse[2];

                kubus.specularReflection.red = specular[0];
                kubus.specularReflection.green = specular[1];
                kubus.specularReflection.blue = specular[2];

                kubus.reflectionCoefficient = reflectionCoefficient;
            }

            applyTranformation(kubus, rotateX, rotateY, rotateZ, scale, centerPunt, eye);

            figuren.push_back(kubus);

            double fractalScale = 3;
            int nr_iterations1 = configuration[figuurNaam]["nrIterations"].as_int_or_die();
            generateMengerSponge(figuren, nr_iterations1, fractalScale);

        }
        else if (configuration[figuurNaam]["type"].as_string_or_die() == "Tetrahedron" || configuration[figuurNaam]["type"].as_string_or_die() == "FractalTetrahedron") {
            Figure tetrahedron = createTetrahedron();

            if (!lighted) {
                tetrahedron.ambientReflection.red = color[0];
                tetrahedron.ambientReflection.blue = color[2];
                tetrahedron.ambientReflection.green = color[1];
            }
            else {
                tetrahedron.ambientReflection.red = ambient[0];
                tetrahedron.ambientReflection.blue = ambient[2];
                tetrahedron.ambientReflection.green = ambient[1];

                tetrahedron.diffuseReflection.red = diffuse[0];
                tetrahedron.diffuseReflection.green = diffuse[1];
                tetrahedron.diffuseReflection.blue = diffuse[2];

                tetrahedron.specularReflection.red = specular[0];
                tetrahedron.specularReflection.green = specular[1];
                tetrahedron.specularReflection.blue = specular[2];

                tetrahedron.reflectionCoefficient = reflectionCoefficient;
            }

            applyTranformation(tetrahedron, rotateX, rotateY, rotateZ, scale, centerPunt, eye);

            if (configuration[figuurNaam]["type"].as_string_or_die() == "FractalTetrahedron") {
                Figures3D fractaalFiguren;
                fractaalFiguren.push_back(tetrahedron);
                double fractalScale = configuration[figuurNaam]["fractalScale"].as_double_or_die();
                int nr_iterations1 = configuration[figuurNaam]["nrIterations"].as_int_or_die();
                generateFractal(tetrahedron, fractaalFiguren, nr_iterations1, fractalScale);
                for (auto it = fractaalFiguren.begin(); it != fractaalFiguren.end(); it++) {
                    figuren.push_back(*it);
                }
            }
            else {
                figuren.push_back(tetrahedron);
            }
        }
        else if (configuration[figuurNaam]["type"].as_string_or_die() == "Icosahedron" || configuration[figuurNaam]["type"].as_string_or_die() == "FractalIcosahedron" || configuration[figuurNaam]["type"].as_string_or_die() == "FractalBuckyBall" || configuration[figuurNaam]["type"].as_string_or_die() == "BuckyBall") {
            Figure icosahedron;
            if (configuration[figuurNaam]["type"].as_string_or_die() == "FractalBuckyBall" || configuration[figuurNaam]["type"].as_string_or_die() == "BuckyBall") {
                icosahedron = createIcosahedron(true);

            }
            else {
                icosahedron = createIcosahedron(false);

            }

            if (!lighted) {
                icosahedron.ambientReflection.red = color[0];
                icosahedron.ambientReflection.blue = color[2];
                icosahedron.ambientReflection.green = color[1];
            }
            else {
                icosahedron.ambientReflection.red = ambient[0];
                icosahedron.ambientReflection.blue = ambient[2];
                icosahedron.ambientReflection.green = ambient[1];

                icosahedron.diffuseReflection.red = diffuse[0];
                icosahedron.diffuseReflection.green = diffuse[1];
                icosahedron.diffuseReflection.blue = diffuse[2];

                icosahedron.specularReflection.red = specular[0];
                icosahedron.specularReflection.green = specular[1];
                icosahedron.specularReflection.blue = specular[2];

                icosahedron.reflectionCoefficient = reflectionCoefficient;
            }

            applyTranformation(icosahedron, rotateX, rotateY, rotateZ, scale, centerPunt, eye);

            if (configuration[figuurNaam]["type"].as_string_or_die() == "FractalIcosahedron" || configuration[figuurNaam]["type"].as_string_or_die() == "FractalBuckyBall") {
                Figures3D fractaalFiguren;
                fractaalFiguren.push_back(icosahedron);
                double fractalScale = configuration[figuurNaam]["fractalScale"].as_double_or_die();
                int nr_iterations1 = configuration[figuurNaam]["nrIterations"].as_int_or_die();
                generateFractal(icosahedron, fractaalFiguren, nr_iterations1, fractalScale);
                for (auto it = fractaalFiguren.begin(); it != fractaalFiguren.end(); it++) {
                    figuren.push_back(*it);
                }
            }
            else {
                figuren.push_back(icosahedron);
            }
        }
        else if (configuration[figuurNaam]["type"].as_string_or_die() == "Dodecahedron" || configuration[figuurNaam]["type"].as_string_or_die() == "FractalDodecahedron") {
            Figure dodecahedron = createDodecahedron();

            if (!lighted) {
                dodecahedron.ambientReflection.red = color[0];
                dodecahedron.ambientReflection.blue = color[2];
                dodecahedron.ambientReflection.green = color[1];
            }
            else {
                dodecahedron.ambientReflection.red = ambient[0];
                dodecahedron.ambientReflection.blue = ambient[2];
                dodecahedron.ambientReflection.green = ambient[1];

                dodecahedron.diffuseReflection.red = diffuse[0];
                dodecahedron.diffuseReflection.green = diffuse[1];
                dodecahedron.diffuseReflection.blue = diffuse[2];

                dodecahedron.specularReflection.red = specular[0];
                dodecahedron.specularReflection.green = specular[1];
                dodecahedron.specularReflection.blue = specular[2];

                dodecahedron.reflectionCoefficient = reflectionCoefficient;
            }

            applyTranformation(dodecahedron, rotateX, rotateY, rotateZ, scale, centerPunt, eye);

            if (configuration[figuurNaam]["type"].as_string_or_die() == "FractalDodecahedron") {
                Figures3D fractaalFiguren;
                fractaalFiguren.push_back(dodecahedron);
                double fractalScale = configuration[figuurNaam]["fractalScale"].as_double_or_die();
                int nr_iterations1 = configuration[figuurNaam]["nrIterations"].as_int_or_die();
                generateFractal(dodecahedron, fractaalFiguren, nr_iterations1, fractalScale);
                for (auto it = fractaalFiguren.begin(); it != fractaalFiguren.end(); it++) {
                    figuren.push_back(*it);
                }
            }
            else {
                figuren.push_back(dodecahedron);
            }
        }
        else if (configuration[figuurNaam]["type"].as_string_or_die() == "Octahedron" || configuration[figuurNaam]["type"].as_string_or_die() == "FractalOctahedron") {
            Figure octahedron = createOctahedron();

            if (!lighted) {
                octahedron.ambientReflection.red = color[0];
                octahedron.ambientReflection.blue = color[2];
                octahedron.ambientReflection.green = color[1];
            }
            else {
                octahedron.ambientReflection.red = ambient[0];
                octahedron.ambientReflection.blue = ambient[2];
                octahedron.ambientReflection.green = ambient[1];

                octahedron.diffuseReflection.red = diffuse[0];
                octahedron.diffuseReflection.green = diffuse[1];
                octahedron.diffuseReflection.blue = diffuse[2];

                octahedron.specularReflection.red = specular[0];
                octahedron.specularReflection.green = specular[1];
                octahedron.specularReflection.blue = specular[2];

                octahedron.reflectionCoefficient = reflectionCoefficient;
            }

            applyTranformation(octahedron, rotateX, rotateY, rotateZ, scale, centerPunt, eye);

            if (configuration[figuurNaam]["type"].as_string_or_die() == "FractalOctahedron") {
                Figures3D fractaalFiguren;
                fractaalFiguren.push_back(octahedron);

                double fractalScale = configuration[figuurNaam]["fractalScale"].as_double_or_die();
                int nr_iterations1 = configuration[figuurNaam]["nrIterations"].as_int_or_die();
                generateFractal(octahedron, fractaalFiguren, nr_iterations1, fractalScale);
                for (auto it = fractaalFiguren.begin(); it != fractaalFiguren.end(); it++) {
                    figuren.push_back(*it);
                }
            }
            else {
                figuren.push_back(octahedron);
            }
        }
        else if (configuration[figuurNaam]["type"].as_string_or_die() == "Cone") {
            double height = configuration[figuurNaam]["height"].as_double_or_die();
            int n = configuration[figuurNaam]["n"].as_int_or_die();
            Figure cone = createCone(n, height);

            if (!lighted) {
                cone.ambientReflection.red = color[0];
                cone.ambientReflection.blue = color[2];
                cone.ambientReflection.green = color[1];
            }
            else {
                cone.ambientReflection.red = ambient[0];
                cone.ambientReflection.blue = ambient[2];
                cone.ambientReflection.green = ambient[1];

                cone.diffuseReflection.red = diffuse[0];
                cone.diffuseReflection.green = diffuse[1];
                cone.diffuseReflection.blue = diffuse[2];

                cone.specularReflection.red = specular[0];
                cone.specularReflection.green = specular[1];
                cone.specularReflection.blue = specular[2];

                cone.reflectionCoefficient = reflectionCoefficient;
            }

            applyTranformation(cone, rotateX, rotateY, rotateZ, scale, centerPunt, eye);

            figuren.push_back(cone);
        }
        else if (configuration[figuurNaam]["type"].as_string_or_die() == "Cylinder") {
            double height = configuration[figuurNaam]["height"].as_double_or_die();
            int n = configuration[figuurNaam]["n"].as_int_or_die();
            Figure cylinder = createCylinder(n, height);

            if (!lighted) {
                cylinder.ambientReflection.red = color[0];
                cylinder.ambientReflection.blue = color[2];
                cylinder.ambientReflection.green = color[1];
            }
            else {
                cylinder.ambientReflection.red = ambient[0];
                cylinder.ambientReflection.blue = ambient[2];
                cylinder.ambientReflection.green = ambient[1];

                cylinder.diffuseReflection.red = diffuse[0];
                cylinder.diffuseReflection.green = diffuse[1];
                cylinder.diffuseReflection.blue = diffuse[2];

                cylinder.specularReflection.red = specular[0];
                cylinder.specularReflection.green = specular[1];
                cylinder.specularReflection.blue = specular[2];

                cylinder.reflectionCoefficient = reflectionCoefficient;
            }

            applyTranformation(cylinder, rotateX, rotateY, rotateZ, scale, centerPunt, eye);

            figuren.push_back(cylinder);
        }
        else if (configuration[figuurNaam]["type"].as_string_or_die() == "Sphere") {
            int radius;
            int n = configuration[figuurNaam]["n"].as_int_or_die();
            Figure sphere = createSphere(radius, n);

            if (!lighted) {
                sphere.ambientReflection.red = color[0];
                sphere.ambientReflection.blue = color[2];
                sphere.ambientReflection.green = color[1];
            }
            else {
                sphere.ambientReflection.red = ambient[0];
                sphere.ambientReflection.blue = ambient[2];
                sphere.ambientReflection.green = ambient[1];

                sphere.diffuseReflection.red = diffuse[0];
                sphere.diffuseReflection.green = diffuse[1];
                sphere.diffuseReflection.blue = diffuse[2];

                sphere.specularReflection.red = specular[0];
                sphere.specularReflection.green = specular[1];
                sphere.specularReflection.blue = specular[2];

                sphere.reflectionCoefficient = reflectionCoefficient;
            }

            applyTranformation(sphere, rotateX, rotateY, rotateZ, scale, centerPunt, eye);

            figuren.push_back(sphere);
        }
        else if (configuration[figuurNaam]["type"].as_string_or_die() == "Torus") {
            int m = configuration[figuurNaam]["m"].as_int_or_die();
            int n = configuration[figuurNaam]["n"].as_int_or_die();
            double r = configuration[figuurNaam]["r"].as_double_or_die();
            double R = configuration[figuurNaam]["R"].as_double_or_die();
            Figure torus = createTorus(r, R, n, m);

            if (!lighted) {
                torus.ambientReflection.red = color[0];
                torus.ambientReflection.blue = color[2];
                torus.ambientReflection.green = color[1];
            }
            else {
                torus.ambientReflection.red = ambient[0];
                torus.ambientReflection.blue = ambient[2];
                torus.ambientReflection.green = ambient[1];

                torus.diffuseReflection.red = diffuse[0];
                torus.diffuseReflection.green = diffuse[1];
                torus.diffuseReflection.blue = diffuse[2];

                torus.specularReflection.red = specular[0];
                torus.specularReflection.green = specular[1];
                torus.specularReflection.blue = specular[2];

                torus.reflectionCoefficient = reflectionCoefficient;
            }

            applyTranformation(torus, rotateX, rotateY, rotateZ, scale, centerPunt, eye);

            figuren.push_back(torus);
        }
        else if (configuration[figuurNaam]["type"].as_string_or_die() == "3DLSystem") {
            string inputfile = configuration[figuurNaam]["inputfile"].as_string_or_die();

            Figure figuur1 = parseFile(inputfile, ambient);

            if (!lighted) {
                figuur1.ambientReflection.red = color[0];
                figuur1.ambientReflection.blue = color[2];
                figuur1.ambientReflection.green = color[1];
            }
            else {
                figuur1.ambientReflection.red = ambient[0];
                figuur1.ambientReflection.blue = ambient[2];
                figuur1.ambientReflection.green = ambient[1];

                figuur1.diffuseReflection.red = diffuse[0];
                figuur1.diffuseReflection.green = diffuse[1];
                figuur1.diffuseReflection.blue = diffuse[2];

                figuur1.specularReflection.red = specular[0];
                figuur1.specularReflection.green = specular[1];
                figuur1.specularReflection.blue = specular[2];

                figuur1.reflectionCoefficient = reflectionCoefficient;
            }

            applyTranformation(figuur1, rotateX, rotateY, rotateZ, scale, centerPunt, eye);
            figuren.push_back(figuur1);
        }
        else if (configuration[figuurNaam]["type"].as_string_or_die() == "ThickLineDrawing" ||
                configuration[figuurNaam]["type"].as_string_or_die() == "ThickCube" ||
                configuration[figuurNaam]["type"].as_string_or_die() == "ThickDodecahedron" ||
                configuration[figuurNaam]["type"].as_string_or_die() == "ThickIcosahedron" ||
                configuration[figuurNaam]["type"].as_string_or_die() == "ThickOctahedron" ||
                configuration[figuurNaam]["type"].as_string_or_die() == "ThickTetrahedron" ||
                configuration[figuurNaam]["type"].as_string_or_die() == "Thick3DLSystem" ||
                configuration[figuurNaam]["type"].as_string_or_die() == "ThickBuckyBall") {
            list<Line2D> lijnen;
            return lijnen;
        }
        else {
            int nrPoints = configuration[figuurNaam]["nrPoints"].as_int_or_die();
            int nrLines = configuration[figuurNaam]["nrLines"].as_int_or_die();

            for (auto j = 0; j < nrPoints; j++) {
                string puntNaam = "point" + to_string(j);
                Vector3D punt;
                vector<double> puntTuple = configuration[figuurNaam][puntNaam].as_double_tuple_or_die();
                punt.x = puntTuple[0];
                punt.y = puntTuple[1];
                punt.z = puntTuple[2];
                figuur.points.push_back(punt);
            }

            for (auto k = 0; k < nrLines; k++) {
                string lijnNaam = "line" + to_string(k);
                Face vlak;
                vector<int> lijn = configuration[figuurNaam][lijnNaam].as_int_tuple_or_die();
                vlak.point_indexes = lijn;
                figuur.faces.push_back(vlak);
            }

            applyTranformation(figuur, rotateX, rotateY, rotateZ, scale, centerPunt, eye);

            figuren.push_back(figuur);
        }
    }

    if (zbufTriag) {
        for (auto it = figuren.begin(); it != figuren.end(); it++) {
            Figure figuur;
            figuur.points = it->points;
            figuur.ambientReflection = it->ambientReflection;
            figuur.diffuseReflection = it->diffuseReflection;
            figuur.specularReflection = it->specularReflection;
            figuur.reflectionCoefficient = it->reflectionCoefficient;

            vector<Face> vlakken = it->faces;
            for (auto it2 = 0; it2 < vlakken.size(); it2++) {
                vector<Face> getrianguleerdeVlakken = triangulate(vlakken[it2]);
                for (auto it3 = 0; it3 < getrianguleerdeVlakken.size(); it3++) {
                    figuur.faces.push_back(getrianguleerdeVlakken[it3]);
                }
            }
            figuren3D.push_back(figuur);
        }
        if (isShadowEnabled) {
            for (auto &licht : lights) {
                if (licht.infinity == false) {
                    Vector3D eyepoint = Vector3D::vector(licht.location);
                    licht.eye = eyePointTrans(eyepoint);

                    Vector3D eye1 = Vector3D::vector(eye[0], eye[1], eye[2]);
                    Matrix inv = eyePointTrans(eye1);
                    inv = Matrix::inv(inv);
                    Figures3D figurenTemp = figuren;

                    for (auto &figuur: figurenTemp) {
                        for (auto it = 0; it < figuur.points.size(); it++) {
                            figuur.points[it] *= inv;
                        }
                    }

                    for (auto &figuur: figurenTemp) {
                        for (auto it = 0; it < figuur.points.size(); it++) {
                            figuur.points[it] *= licht.eye;
                        }
                    }
                    calculateShadowMask(shadowMask, figurenTemp, licht.shadowMask1, licht);
                }
            }
        }

        list<Line2D> lijnen;
        return lijnen;
    }

    list<Line2D> lijnen = doProjection(figuren);
    return lijnen;
}

Matrix L3Dsysteem::eyePointTrans(const Vector3D &eyepoint) {
    double theta;
    double phi;
    double r;
    toPolar(eyepoint, theta, phi, r);

    Matrix V;
    V(1, 1) = -sin(theta);
    V(1, 2) = -cos(theta) * cos(phi);
    V(1, 3) = cos(theta) * sin(phi);
    V(2, 1) = cos(theta);
    V(2, 2) = -sin(theta) * cos(phi);
    V(3, 2) = sin(phi);
    V(2, 3) = sin(theta) * sin(phi);
    V(3,3) = cos(phi);
    V(4, 3) = -r;

    return V;

}

void L3Dsysteem::toPolar(const Vector3D &point, double &theta, double &phi, double &r) {
    r = sqrt(pow((point.x), 2) + pow((point.y), 2) + pow((point.z), 2));
    theta = atan2(point.y, point.x);
    phi = acos(point.z / r);

}

list<Line2D> L3Dsysteem::doProjection(const Figures3D &figuren) {
    list<Line2D> lijnen;

    for (auto it = figuren.begin(); it != figuren.end(); it++) {
        vector<Vector3D> punten = it->points;
        vector<Face> vlakken = it->faces;

        for (auto it2 = 0; it2 < vlakken.size(); it2++) {
            vector<Point2D> lijn;
            vector<double> Z_waarden;
            for (auto it3 = 0; it3 < vlakken[it2].point_indexes.size(); it3++) {
                Point2D punt = doProjectionPoint(punten[vlakken[it2].point_indexes[it3]], 1); // index -1 doen?
                lijn.push_back(punt);

                Z_waarden.push_back(punten[vlakken[it2].point_indexes[it3]].z);
            }

            if (lijn.size() == 2) {
                Line2D lijn1 = Line2D(lijn[0], lijn[1], it->ambientReflection);
                lijn1.z1 = Z_waarden[0];
                lijn1.z2 = Z_waarden[1];
                lijnen.push_back(lijn1);
            }
            else {
                for (auto i = 0; i < lijn.size()-1; i++) {
                    Line2D lijn1 = Line2D(lijn[i], lijn[i+1], it->ambientReflection);
                    lijn1.z1 = Z_waarden[i];
                    lijn1.z2 = Z_waarden[i+1];
                    lijnen.push_back(lijn1);
                }
                Line2D lijn1 = Line2D(lijn[0], lijn[lijn.size()-1], it->ambientReflection);
                lijn1.z1 = Z_waarden[0];
                lijn1.z2 = Z_waarden[Z_waarden.size()-1];
                lijnen.push_back(lijn1);
            }
        }
    }

    return lijnen;
}

Point2D L3Dsysteem::doProjectionPoint(const Vector3D &point, const double d) {
    Point2D punt;
    punt.x = (d * point.x) / (-point.z);
    punt.y = (d * point.y) / (-point.z);
    return punt;
}

void L3Dsysteem::applyTranformation(Figure &figuur, const double &RotateX, const double &RotateY, const double &RotateZ,
                        double &scale, Vector3D &center, const vector<double>& eye) {
    Vector3D eyepoint;
    eyepoint.x = eye[0];
    eyepoint.y = eye[1];
    eyepoint.z = eye[2];

    Matrix TransformatieMatrix = scaleFigure(scale);
    TransformatieMatrix *= rotateX(RotateX);
    TransformatieMatrix *= rotateY(RotateY);
    TransformatieMatrix *= rotateZ(RotateZ);
    TransformatieMatrix *= translate(center);
    TransformatieMatrix *= eyePointTrans(eyepoint);
    for (auto it = 0; it < figuur.points.size(); it++) {
        figuur.points[it] *= TransformatieMatrix;
    }
}


//
Figure createCube() {
    Figure kubus;
    Vector3D punt1;
    punt1.x = 1;
    punt1.y = -1;
    punt1.z = -1;
    Vector3D punt2;
    punt2.x = -1;
    punt2.y = 1;
    punt2.z = -1;
    Vector3D punt3;
    punt3.x = 1;
    punt3.y = 1;
    punt3.z = 1;
    Vector3D punt4;
    punt4.x = -1;
    punt4.y = -1;
    punt4.z = 1;
    Vector3D punt5;
    punt5.x = 1;
    punt5.y = 1;
    punt5.z = -1;
    Vector3D punt6;
    punt6.x = -1;
    punt6.y = -1;
    punt6.z = -1;
    Vector3D punt7;
    punt7.x = 1;
    punt7.y = -1;
    punt7.z = 1;
    Vector3D punt8;
    punt8.x = -1;
    punt8.y = 1;
    punt8.z = 1;
    kubus.points.push_back(punt1);
    kubus.points.push_back(punt2);
    kubus.points.push_back(punt3);
    kubus.points.push_back(punt4);
    kubus.points.push_back(punt5);
    kubus.points.push_back(punt6);
    kubus.points.push_back(punt7);
    kubus.points.push_back(punt8);

    Face vlak1;
    vlak1.point_indexes.push_back(0);
    vlak1.point_indexes.push_back(4);
    vlak1.point_indexes.push_back(2);
    vlak1.point_indexes.push_back(6);

    kubus.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(4);
    vlak1.point_indexes.push_back(1);
    vlak1.point_indexes.push_back(7);
    vlak1.point_indexes.push_back(2);

    kubus.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(1);
    vlak1.point_indexes.push_back(5);
    vlak1.point_indexes.push_back(3);
    vlak1.point_indexes.push_back(7);

    kubus.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(5);
    vlak1.point_indexes.push_back(0);
    vlak1.point_indexes.push_back(6);
    vlak1.point_indexes.push_back(3);

    kubus.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(6);
    vlak1.point_indexes.push_back(2);
    vlak1.point_indexes.push_back(7);
    vlak1.point_indexes.push_back(3);

    kubus.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(0);
    vlak1.point_indexes.push_back(5);
    vlak1.point_indexes.push_back(1);
    vlak1.point_indexes.push_back(4);

    kubus.faces.push_back(vlak1);

    return kubus;
}
//
Figure createTetrahedron() {
    Figure tetrahedron;
    Vector3D punt1;
    punt1.x = 1;
    punt1.y = -1;
    punt1.z = -1;
    Vector3D punt2;
    punt2.x = -1;
    punt2.y = 1;
    punt2.z = -1;
    Vector3D punt3;
    punt3.x = 1;
    punt3.y = 1;
    punt3.z = 1;
    Vector3D punt4;
    punt4.x = -1;
    punt4.y = -1;
    punt4.z = 1;
    tetrahedron.points.push_back(punt1);
    tetrahedron.points.push_back(punt2);
    tetrahedron.points.push_back(punt3);
    tetrahedron.points.push_back(punt4);

    Face vlak1;
    vlak1.point_indexes.push_back(0);
    vlak1.point_indexes.push_back(1);
    vlak1.point_indexes.push_back(2);
    tetrahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(1);
    vlak1.point_indexes.push_back(3);
    vlak1.point_indexes.push_back(2);
    tetrahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(0);
    vlak1.point_indexes.push_back(3);
    vlak1.point_indexes.push_back(1);
    tetrahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(0);
    vlak1.point_indexes.push_back(2);
    vlak1.point_indexes.push_back(3);
    tetrahedron.faces.push_back(vlak1);

    return tetrahedron;
}
//
Figure createOctahedron() {
    Figure octahedron;
    Vector3D punt1;
    punt1.x = 1;
    punt1.y = 0;
    punt1.z = 0;
    Vector3D punt2;
    punt2.x = 0;
    punt2.y = 1;
    punt2.z = 0;
    Vector3D punt3;
    punt3.x = -1;
    punt3.y = 0;
    punt3.z = 0;
    Vector3D punt4;
    punt4.x = 0;
    punt4.y = -1;
    punt4.z = 0;
    Vector3D punt5;
    punt5.x = 0;
    punt5.y = 0;
    punt5.z = -1;
    Vector3D punt6;
    punt6.x = 0;
    punt6.y = 0;
    punt6.z = 1;

    octahedron.points.push_back(punt1);
    octahedron.points.push_back(punt2);
    octahedron.points.push_back(punt3);
    octahedron.points.push_back(punt4);
    octahedron.points.push_back(punt5);
    octahedron.points.push_back(punt6);

    Face vlak1;
    vlak1.point_indexes.push_back(0);
    vlak1.point_indexes.push_back(1);
    vlak1.point_indexes.push_back(5);

    octahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(1);
    vlak1.point_indexes.push_back(2);
    vlak1.point_indexes.push_back(5);

    octahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(2);
    vlak1.point_indexes.push_back(3);
    vlak1.point_indexes.push_back(5);

    octahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(3);
    vlak1.point_indexes.push_back(0);
    vlak1.point_indexes.push_back(5);

    octahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(1);
    vlak1.point_indexes.push_back(0);
    vlak1.point_indexes.push_back(4);

    octahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(2);
    vlak1.point_indexes.push_back(1);
    vlak1.point_indexes.push_back(4);

    octahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(3);
    vlak1.point_indexes.push_back(2);
    vlak1.point_indexes.push_back(4);

    octahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(0);
    vlak1.point_indexes.push_back(3);
    vlak1.point_indexes.push_back(4);

    octahedron.faces.push_back(vlak1);

    return octahedron;
}
//
Figure createIcosahedron(bool bucky) {
    Figure icosahedron;

    for (auto i = 1; i < 13; i++) {
        Vector3D punt1;
        if (i == 1) {
            punt1.x = 0;
            punt1.y = 0;
            punt1.z = sqrt(5)/2;
        }
        else if (i > 1 && i < 7) {
            punt1.x = cos((i-2)*(2*M_PI)/5);
            punt1.y = sin((i-2)*(2*M_PI)/5);
            punt1.z = 0.5;
        }
        else if (i > 6 && i < 12) {
            punt1.x = cos((M_PI/5)+(i-7)*(2*M_PI)/5);
            punt1.y = sin((M_PI/5)+(i-7)*(2*M_PI)/5);
            punt1.z = -0.5;
        }
        else if (i == 12) {
            punt1.x = 0;
            punt1.y = 0;
            punt1.z = -sqrt(5)/2;
        }
        icosahedron.points.push_back(punt1);
    }
    if (bucky) {
        Vector3D punt1 = icosahedron.points[0] + (icosahedron.points[1]-icosahedron.points[0])/3;
        Vector3D punt2 = icosahedron.points[0] + (icosahedron.points[1]-icosahedron.points[0])*2/3;

        Vector3D punt3 = icosahedron.points[1] + (icosahedron.points[2]-icosahedron.points[1])/3;
        Vector3D punt4 = icosahedron.points[1] + (icosahedron.points[2]-icosahedron.points[1])*2/3;

        Vector3D punt5 = icosahedron.points[2] + (icosahedron.points[0]-icosahedron.points[2])/3;
        Vector3D punt6 = icosahedron.points[2] + (icosahedron.points[0]-icosahedron.points[2])*2/3;
        //
        Vector3D punt7 = icosahedron.points[2] + (icosahedron.points[3]-icosahedron.points[2])/3;
        Vector3D punt8 = icosahedron.points[2] + (icosahedron.points[3]-icosahedron.points[2])*2/3;

        Vector3D punt9 = icosahedron.points[3] + (icosahedron.points[0]-icosahedron.points[3])/3;
        Vector3D punt10 = icosahedron.points[3] + (icosahedron.points[0]-icosahedron.points[3])*2/3;
        //
        Vector3D punt11 = icosahedron.points[3] + (icosahedron.points[4]-icosahedron.points[3])/3;
        Vector3D punt12 = icosahedron.points[3] + (icosahedron.points[4]-icosahedron.points[3])*2/3;

        Vector3D punt13 = icosahedron.points[4] + (icosahedron.points[0]-icosahedron.points[4])/3;
        Vector3D punt14 = icosahedron.points[4] + (icosahedron.points[0]-icosahedron.points[4])*2/3;
        //
        Vector3D punt15 = icosahedron.points[4] + (icosahedron.points[5]-icosahedron.points[4])/3;
        Vector3D punt16 = icosahedron.points[4] + (icosahedron.points[5]-icosahedron.points[4])*2/3;

        Vector3D punt17 = icosahedron.points[5] + (icosahedron.points[0]-icosahedron.points[5])/3;
        Vector3D punt18 = icosahedron.points[5] + (icosahedron.points[0]-icosahedron.points[5])*2/3;
        //
        Vector3D punt19 = icosahedron.points[5] + (icosahedron.points[1]-icosahedron.points[5])/3; // bug gevonden
        Vector3D punt20 = icosahedron.points[5] + (icosahedron.points[1]-icosahedron.points[5])*2/3;
        // tot hier zijn alle driehoeken vanboven gedaan
        //
        //
        Vector3D punt21 = icosahedron.points[1] + (icosahedron.points[6]-icosahedron.points[1])/3;
        Vector3D punt22 = icosahedron.points[1] + (icosahedron.points[6]-icosahedron.points[1])*2/3;

        Vector3D punt23 = icosahedron.points[6] + (icosahedron.points[2]-icosahedron.points[6])/3;
        Vector3D punt24 = icosahedron.points[6] + (icosahedron.points[2]-icosahedron.points[6])*2/3;
        //
        Vector3D punt25 = icosahedron.points[2] + (icosahedron.points[7]-icosahedron.points[2])/3;
        Vector3D punt26 = icosahedron.points[2] + (icosahedron.points[7]-icosahedron.points[2])*2/3;

        Vector3D punt27 = icosahedron.points[7] + (icosahedron.points[3]-icosahedron.points[7])/3;
        Vector3D punt28 = icosahedron.points[7] + (icosahedron.points[3]-icosahedron.points[7])*2/3;
        //
        Vector3D punt29 = icosahedron.points[3] + (icosahedron.points[8]-icosahedron.points[3])/3;
        Vector3D punt30 = icosahedron.points[3] + (icosahedron.points[8]-icosahedron.points[3])*2/3;

        Vector3D punt31 = icosahedron.points[8] + (icosahedron.points[4]-icosahedron.points[8])/3;
        Vector3D punt32 = icosahedron.points[8] + (icosahedron.points[4]-icosahedron.points[8])*2/3;
        //
        Vector3D punt33 = icosahedron.points[4] + (icosahedron.points[9]-icosahedron.points[4])/3;
        Vector3D punt34 = icosahedron.points[4] + (icosahedron.points[9]-icosahedron.points[4])*2/3;

        Vector3D punt35 = icosahedron.points[9] + (icosahedron.points[5]-icosahedron.points[9])/3;
        Vector3D punt36 = icosahedron.points[9] + (icosahedron.points[5]-icosahedron.points[9])*2/3;
        //
        Vector3D punt37 = icosahedron.points[5] + (icosahedron.points[10]-icosahedron.points[5])/3;
        Vector3D punt38 = icosahedron.points[5] + (icosahedron.points[10]-icosahedron.points[5])*2/3;

        Vector3D punt39 = icosahedron.points[10] + (icosahedron.points[1]-icosahedron.points[10])/3;
        Vector3D punt40 = icosahedron.points[10] + (icosahedron.points[1]-icosahedron.points[10])*2/3;
        // tot hier zijn ook alle driehoeken in het midden gedaan
        //
        Vector3D punt41 = icosahedron.points[10] + (icosahedron.points[6]-icosahedron.points[10])/3;
        Vector3D punt42 = icosahedron.points[10] + (icosahedron.points[6]-icosahedron.points[10])*2/3;

        Vector3D punt43 = icosahedron.points[10] + (icosahedron.points[11]-icosahedron.points[10])/3;
        Vector3D punt44 = icosahedron.points[10] + (icosahedron.points[11]-icosahedron.points[10])*2/3;

        Vector3D punt45 = icosahedron.points[11] + (icosahedron.points[6]-icosahedron.points[11])/3;
        Vector3D punt46 = icosahedron.points[11] + (icosahedron.points[6]-icosahedron.points[11])*2/3;
        //
        Vector3D punt47 = icosahedron.points[11] + (icosahedron.points[7]-icosahedron.points[11])/3;
        Vector3D punt48 = icosahedron.points[11] + (icosahedron.points[7]-icosahedron.points[11])*2/3;
        //
        Vector3D punt49 = icosahedron.points[7] + (icosahedron.points[6]-icosahedron.points[7])/3;
        Vector3D punt50 = icosahedron.points[7] + (icosahedron.points[6]-icosahedron.points[7])*2/3;

        Vector3D punt51 = icosahedron.points[7] + (icosahedron.points[8]-icosahedron.points[7])/3;
        Vector3D punt52 = icosahedron.points[7] + (icosahedron.points[8]-icosahedron.points[7])*2/3;
        //
        Vector3D punt53 = icosahedron.points[8] + (icosahedron.points[11]-icosahedron.points[8])/3;
        Vector3D punt54 = icosahedron.points[8] + (icosahedron.points[11]-icosahedron.points[8])*2/3;

        Vector3D punt55 = icosahedron.points[8] + (icosahedron.points[9]-icosahedron.points[8])/3;
        Vector3D punt56 = icosahedron.points[8] + (icosahedron.points[9]-icosahedron.points[8])*2/3;
        //
        Vector3D punt57 = icosahedron.points[9] + (icosahedron.points[11]-icosahedron.points[9])/3;
        Vector3D punt58 = icosahedron.points[9] + (icosahedron.points[11]-icosahedron.points[9])*2/3;

        Vector3D punt59 = icosahedron.points[9] + (icosahedron.points[10]-icosahedron.points[9])/3;
        Vector3D punt60 = icosahedron.points[9] + (icosahedron.points[10]-icosahedron.points[9])*2/3;
        // ik heb hierboven geen for loop gebruikt omdat ik dit deed m.b.v. een tekening en ik wou zeker checken dat alle punten juist zijn, ook voor de vlakken later
        // vlakken opstellen
        icosahedron.points.clear();
        icosahedron.points.push_back(punt1);
        icosahedron.points.push_back(punt2);
        icosahedron.points.push_back(punt3);
        icosahedron.points.push_back(punt4);
        icosahedron.points.push_back(punt5);
        icosahedron.points.push_back(punt6);
        icosahedron.points.push_back(punt7);
        icosahedron.points.push_back(punt8);
        icosahedron.points.push_back(punt9);
        icosahedron.points.push_back(punt10);
        icosahedron.points.push_back(punt11);
        icosahedron.points.push_back(punt12);
        icosahedron.points.push_back(punt13);
        icosahedron.points.push_back(punt14);
        icosahedron.points.push_back(punt15);
        icosahedron.points.push_back(punt16);
        icosahedron.points.push_back(punt17);
        icosahedron.points.push_back(punt18);
        icosahedron.points.push_back(punt19);
        icosahedron.points.push_back(punt20);
        icosahedron.points.push_back(punt21);
        icosahedron.points.push_back(punt22);
        icosahedron.points.push_back(punt23);
        icosahedron.points.push_back(punt24);
        icosahedron.points.push_back(punt25);
        icosahedron.points.push_back(punt26);
        icosahedron.points.push_back(punt27);
        icosahedron.points.push_back(punt28);
        icosahedron.points.push_back(punt29);
        icosahedron.points.push_back(punt30);
        icosahedron.points.push_back(punt31);
        icosahedron.points.push_back(punt32);
        icosahedron.points.push_back(punt33);
        icosahedron.points.push_back(punt34);
        icosahedron.points.push_back(punt35);
        icosahedron.points.push_back(punt36);
        icosahedron.points.push_back(punt37);
        icosahedron.points.push_back(punt38);
        icosahedron.points.push_back(punt39);
        icosahedron.points.push_back(punt40);
        icosahedron.points.push_back(punt41);
        icosahedron.points.push_back(punt42);
        icosahedron.points.push_back(punt43);
        icosahedron.points.push_back(punt44);
        icosahedron.points.push_back(punt45);
        icosahedron.points.push_back(punt46);
        icosahedron.points.push_back(punt47);
        icosahedron.points.push_back(punt48);
        icosahedron.points.push_back(punt49);
        icosahedron.points.push_back(punt50);
        icosahedron.points.push_back(punt51);
        icosahedron.points.push_back(punt52);
        icosahedron.points.push_back(punt53);
        icosahedron.points.push_back(punt54);
        icosahedron.points.push_back(punt55);
        icosahedron.points.push_back(punt56);
        icosahedron.points.push_back(punt57);
        icosahedron.points.push_back(punt58);
        icosahedron.points.push_back(punt59);
        icosahedron.points.push_back(punt60);

        Face vijfhoek;
        vijfhoek.point_indexes.push_back(0);
        vijfhoek.point_indexes.push_back(5);
        vijfhoek.point_indexes.push_back(9);
        vijfhoek.point_indexes.push_back(13);
        vijfhoek.point_indexes.push_back(17);


        Face zeshoek1; //
        zeshoek1.point_indexes.push_back(0);
        zeshoek1.point_indexes.push_back(1);
        zeshoek1.point_indexes.push_back(2);
        zeshoek1.point_indexes.push_back(3);
        zeshoek1.point_indexes.push_back(4);
        zeshoek1.point_indexes.push_back(5);

        Face zeshoek2; //
        zeshoek2.point_indexes.push_back(5);
        zeshoek2.point_indexes.push_back(4);
        zeshoek2.point_indexes.push_back(6);
        zeshoek2.point_indexes.push_back(7);
        zeshoek2.point_indexes.push_back(8);
        zeshoek2.point_indexes.push_back(9);

        Face zeshoek3; //
        zeshoek3.point_indexes.push_back(9);
        zeshoek3.point_indexes.push_back(8);
        zeshoek3.point_indexes.push_back(10);
        zeshoek3.point_indexes.push_back(11);
        zeshoek3.point_indexes.push_back(12);
        zeshoek3.point_indexes.push_back(13);

        Face zeshoek4; //
        zeshoek4.point_indexes.push_back(13);
        zeshoek4.point_indexes.push_back(12);
        zeshoek4.point_indexes.push_back(14);
        zeshoek4.point_indexes.push_back(15);
        zeshoek4.point_indexes.push_back(16);
        zeshoek4.point_indexes.push_back(17);

        Face zeshoek5; //
        zeshoek5.point_indexes.push_back(17);
        zeshoek5.point_indexes.push_back(16);
        zeshoek5.point_indexes.push_back(18);
        zeshoek5.point_indexes.push_back(19);
        zeshoek5.point_indexes.push_back(1);
        zeshoek5.point_indexes.push_back(0);


        Face vijfhoek1;
        vijfhoek1.point_indexes.push_back(1);
        vijfhoek1.point_indexes.push_back(19); ///
        vijfhoek1.point_indexes.push_back(39);
        vijfhoek1.point_indexes.push_back(20);
        vijfhoek1.point_indexes.push_back(2);

        Face vijfhoek2;
        vijfhoek2.point_indexes.push_back(4);
        vijfhoek2.point_indexes.push_back(3);
        vijfhoek2.point_indexes.push_back(23);
        vijfhoek2.point_indexes.push_back(24);
        vijfhoek2.point_indexes.push_back(6);

        Face vijfhoek3;
        vijfhoek3.point_indexes.push_back(8);
        vijfhoek3.point_indexes.push_back(7);
        vijfhoek3.point_indexes.push_back(27);
        vijfhoek3.point_indexes.push_back(28);
        vijfhoek3.point_indexes.push_back(10);

        Face vijfhoek4;
        vijfhoek4.point_indexes.push_back(12);
        vijfhoek4.point_indexes.push_back(11);
        vijfhoek4.point_indexes.push_back(31);
        vijfhoek4.point_indexes.push_back(32);
        vijfhoek4.point_indexes.push_back(14);

        Face vijfhoek5;
        vijfhoek5.point_indexes.push_back(16);
        vijfhoek5.point_indexes.push_back(15);
        vijfhoek5.point_indexes.push_back(35);
        vijfhoek5.point_indexes.push_back(36);
        vijfhoek5.point_indexes.push_back(18); ///


        Face zeshoek6;
        zeshoek6.point_indexes.push_back(39);
        zeshoek6.point_indexes.push_back(38);
        zeshoek6.point_indexes.push_back(40);
        zeshoek6.point_indexes.push_back(41);
        zeshoek6.point_indexes.push_back(21);
        zeshoek6.point_indexes.push_back(20);

        Face zeshoek7;
        zeshoek7.point_indexes.push_back(3);
        zeshoek7.point_indexes.push_back(2);
        zeshoek7.point_indexes.push_back(20);
        zeshoek7.point_indexes.push_back(21);
        zeshoek7.point_indexes.push_back(22);
        zeshoek7.point_indexes.push_back(23);

        Face zeshoek8; ///
        zeshoek8.point_indexes.push_back(23);
        zeshoek8.point_indexes.push_back(22);
        zeshoek8.point_indexes.push_back(49);
        zeshoek8.point_indexes.push_back(48);
        zeshoek8.point_indexes.push_back(25);
        zeshoek8.point_indexes.push_back(24);

        Face zeshoek9;
        zeshoek9.point_indexes.push_back(7);
        zeshoek9.point_indexes.push_back(6);
        zeshoek9.point_indexes.push_back(24);///
        zeshoek9.point_indexes.push_back(25);
        zeshoek9.point_indexes.push_back(26);
        zeshoek9.point_indexes.push_back(27);

        Face zeshoek10;
        zeshoek10.point_indexes.push_back(27);
        zeshoek10.point_indexes.push_back(26);
        zeshoek10.point_indexes.push_back(50);
        zeshoek10.point_indexes.push_back(51);
        zeshoek10.point_indexes.push_back(29);
        zeshoek10.point_indexes.push_back(28);

        Face zeshoek11;
        zeshoek11.point_indexes.push_back(28);
        zeshoek11.point_indexes.push_back(29);
        zeshoek11.point_indexes.push_back(30);
        zeshoek11.point_indexes.push_back(31);
        zeshoek11.point_indexes.push_back(11);
        zeshoek11.point_indexes.push_back(10);

        Face zeshoek12;
        zeshoek12.point_indexes.push_back(31); ///
        zeshoek12.point_indexes.push_back(30);
        zeshoek12.point_indexes.push_back(54);
        zeshoek12.point_indexes.push_back(55);
        zeshoek12.point_indexes.push_back(33);
        zeshoek12.point_indexes.push_back(32);

        Face zeshoek13;
        zeshoek13.point_indexes.push_back(32);
        zeshoek13.point_indexes.push_back(33);
        zeshoek13.point_indexes.push_back(34);
        zeshoek13.point_indexes.push_back(35);
        zeshoek13.point_indexes.push_back(15);
        zeshoek13.point_indexes.push_back(14);

        Face zeshoek14;
        zeshoek14.point_indexes.push_back(35);
        zeshoek14.point_indexes.push_back(34);
        zeshoek14.point_indexes.push_back(58);
        zeshoek14.point_indexes.push_back(59);
        zeshoek14.point_indexes.push_back(37);
        zeshoek14.point_indexes.push_back(36);

        Face zeshoek15;
        zeshoek15.point_indexes.push_back(19);
        zeshoek15.point_indexes.push_back(18);
        zeshoek15.point_indexes.push_back(36);
        zeshoek15.point_indexes.push_back(37);
        zeshoek15.point_indexes.push_back(38);
        zeshoek15.point_indexes.push_back(39);

        Face vijfhoek6;
        vijfhoek6.point_indexes.push_back(21);
        vijfhoek6.point_indexes.push_back(41);
        vijfhoek6.point_indexes.push_back(45);
        vijfhoek6.point_indexes.push_back(49);
        vijfhoek6.point_indexes.push_back(22);

        Face vijfhoek7;
        vijfhoek7.point_indexes.push_back(25);
        vijfhoek7.point_indexes.push_back(48);
        vijfhoek7.point_indexes.push_back(47);
        vijfhoek7.point_indexes.push_back(50);
        vijfhoek7.point_indexes.push_back(26);

        Face vijfhoek8;
        vijfhoek8.point_indexes.push_back(51); ///
        vijfhoek8.point_indexes.push_back(52);
        vijfhoek8.point_indexes.push_back(54);
        vijfhoek8.point_indexes.push_back(30);
        vijfhoek8.point_indexes.push_back(29);

        Face vijfhoek9;
        vijfhoek9.point_indexes.push_back(33);
        vijfhoek9.point_indexes.push_back(55);
        vijfhoek9.point_indexes.push_back(56);
        vijfhoek9.point_indexes.push_back(58);
        vijfhoek9.point_indexes.push_back(34);

        Face vijfhoek10;
        vijfhoek10.point_indexes.push_back(38);
        vijfhoek10.point_indexes.push_back(37);
        vijfhoek10.point_indexes.push_back(59);
        vijfhoek10.point_indexes.push_back(42);
        vijfhoek10.point_indexes.push_back(40);

        Face vijfhoek11;
        vijfhoek11.point_indexes.push_back(43);
        vijfhoek11.point_indexes.push_back(44);
        vijfhoek11.point_indexes.push_back(46);
        vijfhoek11.point_indexes.push_back(53);
        vijfhoek11.point_indexes.push_back(57);

        Face zeshoek16;
        zeshoek16.point_indexes.push_back(41);
        zeshoek16.point_indexes.push_back(40);
        zeshoek16.point_indexes.push_back(42);
        zeshoek16.point_indexes.push_back(43);
        zeshoek16.point_indexes.push_back(44);
        zeshoek16.point_indexes.push_back(45);

        Face zeshoek17;
        zeshoek17.point_indexes.push_back(48);
        zeshoek17.point_indexes.push_back(49);
        zeshoek17.point_indexes.push_back(45);
        zeshoek17.point_indexes.push_back(44);
        zeshoek17.point_indexes.push_back(46);
        zeshoek17.point_indexes.push_back(47);

        Face zeshoek18;
        zeshoek18.point_indexes.push_back(51);
        zeshoek18.point_indexes.push_back(50);
        zeshoek18.point_indexes.push_back(47);
        zeshoek18.point_indexes.push_back(46);
        zeshoek18.point_indexes.push_back(53);
        zeshoek18.point_indexes.push_back(52);

        Face zeshoek19;
        zeshoek19.point_indexes.push_back(55);
        zeshoek19.point_indexes.push_back(54);
        zeshoek19.point_indexes.push_back(52);
        zeshoek19.point_indexes.push_back(56);
        zeshoek19.point_indexes.push_back(57);
        zeshoek19.point_indexes.push_back(56);

        Face zeshoek20;
        zeshoek20.point_indexes.push_back(59);
        zeshoek20.point_indexes.push_back(58);
        zeshoek20.point_indexes.push_back(56);
        zeshoek20.point_indexes.push_back(57);
        zeshoek20.point_indexes.push_back(43);
        zeshoek20.point_indexes.push_back(42);

        icosahedron.faces.push_back(vijfhoek);
        icosahedron.faces.push_back(vijfhoek1);
        icosahedron.faces.push_back(vijfhoek2);
        icosahedron.faces.push_back(vijfhoek3);
        icosahedron.faces.push_back(vijfhoek4);
        icosahedron.faces.push_back(vijfhoek5);
        icosahedron.faces.push_back(vijfhoek6);
        icosahedron.faces.push_back(vijfhoek7);
        icosahedron.faces.push_back(vijfhoek8);
        icosahedron.faces.push_back(vijfhoek9);
        icosahedron.faces.push_back(vijfhoek10);
        icosahedron.faces.push_back(vijfhoek11);

        icosahedron.faces.push_back(zeshoek1);
        icosahedron.faces.push_back(zeshoek2);
        icosahedron.faces.push_back(zeshoek3);
        icosahedron.faces.push_back(zeshoek4);
        icosahedron.faces.push_back(zeshoek5);
        icosahedron.faces.push_back(zeshoek6);
        icosahedron.faces.push_back(zeshoek7);
        icosahedron.faces.push_back(zeshoek8);
        icosahedron.faces.push_back(zeshoek9);
        icosahedron.faces.push_back(zeshoek10);
        icosahedron.faces.push_back(zeshoek11);
        icosahedron.faces.push_back(zeshoek12);
        icosahedron.faces.push_back(zeshoek13);
        icosahedron.faces.push_back(zeshoek14);
        icosahedron.faces.push_back(zeshoek15);
        icosahedron.faces.push_back(zeshoek16);
        icosahedron.faces.push_back(zeshoek17);
        icosahedron.faces.push_back(zeshoek18);
        icosahedron.faces.push_back(zeshoek19);
        icosahedron.faces.push_back(zeshoek20);


    }
    else {
        // vlak 1
        Face vlak1;
        vlak1.point_indexes.push_back(0);
        vlak1.point_indexes.push_back(1);
        vlak1.point_indexes.push_back(2);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 2
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(0);
        vlak1.point_indexes.push_back(2);
        vlak1.point_indexes.push_back(3);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 3
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(0);
        vlak1.point_indexes.push_back(3);
        vlak1.point_indexes.push_back(4);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 4
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(0);
        vlak1.point_indexes.push_back(4);
        vlak1.point_indexes.push_back(5);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 5
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(0);
        vlak1.point_indexes.push_back(5);
        vlak1.point_indexes.push_back(1);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 6
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(1);
        vlak1.point_indexes.push_back(6);
        vlak1.point_indexes.push_back(2);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 7
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(2);
        vlak1.point_indexes.push_back(6);
        vlak1.point_indexes.push_back(7);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 8
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(2);
        vlak1.point_indexes.push_back(7);
        vlak1.point_indexes.push_back(3);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 9
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(3);
        vlak1.point_indexes.push_back(7);
        vlak1.point_indexes.push_back(8);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 10
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(3);
        vlak1.point_indexes.push_back(8);
        vlak1.point_indexes.push_back(4);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 11
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(4);
        vlak1.point_indexes.push_back(8);
        vlak1.point_indexes.push_back(9);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 12
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(4);
        vlak1.point_indexes.push_back(9);
        vlak1.point_indexes.push_back(5);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 13
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(5);
        vlak1.point_indexes.push_back(9);
        vlak1.point_indexes.push_back(10);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 14
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(5);
        vlak1.point_indexes.push_back(10);
        vlak1.point_indexes.push_back(1);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 15
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(1);
        vlak1.point_indexes.push_back(10);
        vlak1.point_indexes.push_back(6);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 16
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(11);
        vlak1.point_indexes.push_back(7);
        vlak1.point_indexes.push_back(6);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 17
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(11);
        vlak1.point_indexes.push_back(8);
        vlak1.point_indexes.push_back(7);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 18
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(11);
        vlak1.point_indexes.push_back(9);
        vlak1.point_indexes.push_back(8);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 19
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(11);
        vlak1.point_indexes.push_back(10);
        vlak1.point_indexes.push_back(9);

        icosahedron.faces.push_back(vlak1);
        //
        // vlak 20
        vlak1.point_indexes.clear();
        vlak1.point_indexes.push_back(11);
        vlak1.point_indexes.push_back(6);
        vlak1.point_indexes.push_back(10);

        icosahedron.faces.push_back(vlak1);
    }

    return icosahedron;
}
//
Figure createDodecahedron() {
    Figure dodecahedron;
    Figure icosahedron = createIcosahedron(false);

    for (auto i = 0; i < icosahedron.faces.size(); i++) {

        vector<int> puntIndexen = icosahedron.faces[i].point_indexes;

        Vector3D puntA = icosahedron.points[puntIndexen[0]];
        Vector3D puntB = icosahedron.points[puntIndexen[1]];
        Vector3D puntC = icosahedron.points[puntIndexen[2]];

        Vector3D punt1;

        punt1.x = (puntA.x + puntB.x + puntC.x)/3;
        punt1.y = (puntA.y + puntB.y + puntC.y)/3;
        punt1.z = (puntA.z + puntB.z + puntC.z)/3;

        dodecahedron.points.push_back(punt1);
    }

    Face vlak1;
    vlak1.point_indexes.push_back(0);
    vlak1.point_indexes.push_back(1);
    vlak1.point_indexes.push_back(2);
    vlak1.point_indexes.push_back(3);
    vlak1.point_indexes.push_back(4);


    dodecahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(0);
    vlak1.point_indexes.push_back(5);
    vlak1.point_indexes.push_back(6);
    vlak1.point_indexes.push_back(7);
    vlak1.point_indexes.push_back(1);


    dodecahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(1);
    vlak1.point_indexes.push_back(7);
    vlak1.point_indexes.push_back(8);
    vlak1.point_indexes.push_back(9);
    vlak1.point_indexes.push_back(2);

    dodecahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(2);
    vlak1.point_indexes.push_back(9);
    vlak1.point_indexes.push_back(10);
    vlak1.point_indexes.push_back(11);
    vlak1.point_indexes.push_back(3);

    dodecahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(3);
    vlak1.point_indexes.push_back(11);
    vlak1.point_indexes.push_back(12);
    vlak1.point_indexes.push_back(13);
    vlak1.point_indexes.push_back(4);

    dodecahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(4);
    vlak1.point_indexes.push_back(13);
    vlak1.point_indexes.push_back(14);
    vlak1.point_indexes.push_back(5);
    vlak1.point_indexes.push_back(0);

    dodecahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(19);
    vlak1.point_indexes.push_back(18);
    vlak1.point_indexes.push_back(17);
    vlak1.point_indexes.push_back(16);
    vlak1.point_indexes.push_back(15);

    dodecahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(19);
    vlak1.point_indexes.push_back(14);
    vlak1.point_indexes.push_back(13);
    vlak1.point_indexes.push_back(12);
    vlak1.point_indexes.push_back(18);

    dodecahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(18);
    vlak1.point_indexes.push_back(12);
    vlak1.point_indexes.push_back(11);
    vlak1.point_indexes.push_back(10);
    vlak1.point_indexes.push_back(17);

    dodecahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(17);
    vlak1.point_indexes.push_back(10);
    vlak1.point_indexes.push_back(9);
    vlak1.point_indexes.push_back(8);
    vlak1.point_indexes.push_back(16);

    dodecahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(16);
    vlak1.point_indexes.push_back(8);
    vlak1.point_indexes.push_back(7);
    vlak1.point_indexes.push_back(6);
    vlak1.point_indexes.push_back(15);

    dodecahedron.faces.push_back(vlak1);

    vlak1.point_indexes.clear();
    vlak1.point_indexes.push_back(15);
    vlak1.point_indexes.push_back(6);
    vlak1.point_indexes.push_back(5);
    vlak1.point_indexes.push_back(14);
    vlak1.point_indexes.push_back(19);

    dodecahedron.faces.push_back(vlak1);

    return dodecahedron;
}
//
Figure createCylinder(const int n, const double h) {
    Figure cylinder;
    for (auto i = 0; i < n; i++) {
        Vector3D punt;
        punt.x = cos(2*i*M_PI/n);
        punt.y = sin(2*i*M_PI/n);
        punt.z = 0;
        cylinder.points.push_back(punt);
    }
    for (auto i = 0; i < n; i++) {
        Vector3D punt;
        punt.x = cos(2*i*M_PI/n);
        punt.y = sin(2*i*M_PI/n);
        punt.z = h;
        cylinder.points.push_back(punt);
    }
    // zijvlakken
    for (auto i = 0; i < n; i++) {
        Face vlak;
        vlak.point_indexes.push_back(i);
        vlak.point_indexes.push_back((i+1)%(n));
        vlak.point_indexes.push_back((i+1)%n+n);
        vlak.point_indexes.push_back(n+i);
        cylinder.faces.push_back(vlak);
    }
    // grondvlak
    Face vlak;
    for (auto i = 0; i <= n-1; i++) { // for (auto i = n-1; i >= 0; i--)
        vlak.point_indexes.push_back(i);
    }

    cylinder.faces.push_back(vlak);
    vlak.point_indexes.clear();
    // bovenvlak
    for (auto i = 0; i <= n-1; i++) {
        vlak.point_indexes.push_back(n+i);
    }

    cylinder.faces.push_back(vlak);

    return cylinder;
}
//
Figure createCone(const int n, const double h) {
    Figure cone;
    for (auto i = 0; i < n; i++) {
        Vector3D punt;
        punt.x = cos(2*i*M_PI/n);
        punt.y = sin(2*i*M_PI/n);
        punt.z = 0;
        cone.points.push_back(punt);
    }
    Vector3D puntBoven;
    puntBoven.x = 0;
    puntBoven.y = 0;
    puntBoven.z = h;
    cone.points.push_back(puntBoven);

    for (auto i = 0; i < n; i++) {
        Face vlak;
        vlak.point_indexes.push_back(i);
        vlak.point_indexes.push_back((i+1)%(n));
        vlak.point_indexes.push_back(n);
        cone.faces.push_back(vlak);
    }
    Face vlak;
    for (auto i = 0; i <= n-1; i++) { //auto i = n-1; i >= 0; i--
        vlak.point_indexes.push_back(i);
    }
    cone.faces.push_back(vlak);

    return cone;
}
//
Figure createSphere(const double radius, const int n) {
    Figure icosahedron = createIcosahedron(false);

    for (auto i = 0; i < n; i++) {
        Figure icosahedronVerfijnd;

        for (auto j = 0; j < icosahedron.faces.size(); j++) {
            vector<int> puntIndexen = icosahedron.faces[j].point_indexes;

            Vector3D puntA = icosahedron.points[puntIndexen[0]];
            Vector3D puntB = icosahedron.points[puntIndexen[1]];
            Vector3D puntC = icosahedron.points[puntIndexen[2]];

            Vector3D puntD = (puntA + puntB)/2; // (A + B) / 2

            Vector3D puntE = (puntA + puntC)/2; // (A + C) / 2

            Vector3D puntF = (puntB + puntC)/2; // (B + C) / 2

            icosahedronVerfijnd.points.push_back(puntA);
            icosahedronVerfijnd.points.push_back(puntD);
            icosahedronVerfijnd.points.push_back(puntB);
            icosahedronVerfijnd.points.push_back(puntF);
            icosahedronVerfijnd.points.push_back(puntC);
            icosahedronVerfijnd.points.push_back(puntE);

            Face driehoek; // + (j*6) omdat we telkens 6 punten maken met driehoek, voor de volgende faces
            driehoek.point_indexes.push_back(0+(j*6)); // A
            driehoek.point_indexes.push_back(1+(j*6)); // D
            driehoek.point_indexes.push_back(5+(j*6)); // E
            icosahedronVerfijnd.faces.push_back(driehoek);
            driehoek.point_indexes.clear();

            driehoek.point_indexes.push_back(2+(j*6)); // B
            driehoek.point_indexes.push_back(3+(j*6)); // F
            driehoek.point_indexes.push_back(1+(j*6)); // D
            icosahedronVerfijnd.faces.push_back(driehoek);
            driehoek.point_indexes.clear();

            driehoek.point_indexes.push_back(4+(j*6)); // C
            driehoek.point_indexes.push_back(5+(j*6)); // E
            driehoek.point_indexes.push_back(3+(j*6)); // F
            icosahedronVerfijnd.faces.push_back(driehoek);
            driehoek.point_indexes.clear();

            driehoek.point_indexes.push_back(1+(j*6)); // D
            driehoek.point_indexes.push_back(3+(j*6)); // F
            driehoek.point_indexes.push_back(5+(j*6)); // E
            icosahedronVerfijnd.faces.push_back(driehoek);
            driehoek.point_indexes.clear();

        }
        icosahedron = icosahedronVerfijnd;
    }

    //herschalen punten
    for (auto i = 0; i < icosahedron.points.size(); i++) {
        Vector3D oudPunt = icosahedron.points[i];
        double r = sqrt(pow(oudPunt.x, 2) + pow(oudPunt.y, 2) + pow(oudPunt.z, 2));
        icosahedron.points[i].x = icosahedron.points[i].x/r;
        icosahedron.points[i].y = icosahedron.points[i].y/r;
        icosahedron.points[i].z = icosahedron.points[i].z/r;
    }
    return icosahedron;
}
//
Figure createTorus(const double r, const double R, const int n, const int m) {
    Figure torus;

    // punten
    for (auto i = 0; i < n; i++) {
        for (auto j = 0; j < m; j++) {
            double u = 2*i*M_PI/n;
            double v = 2*j*M_PI/m;
            Vector3D punt;
            punt.x = (R + r*cos(v))*cos(u);
            punt.y = (R + r*cos(v))*sin(u);
            punt.z = r*sin(v);
            torus.points.push_back(punt);

        }
    }

    for (auto i = 0; i < n; i++) {
        for (auto j = 0; j < m; j++) {
            Face vlak;
            vlak.point_indexes.push_back(i*m+j);
            vlak.point_indexes.push_back(((i+1)%(n))*m+j);
            vlak.point_indexes.push_back(((i+1)%(n))*m+(j+1)%m);
            vlak.point_indexes.push_back(i*m+(j+1)%m);

            torus.faces.push_back(vlak);
        }
    }

    return torus;
}

Figure L3Dsysteem::parseFile(const string &filenaam, const vector<double>& ambientReflection) {

    LParser::LSystem3D l_system;

    std::ifstream input_stream(filenaam);
    input_stream >> l_system;
    input_stream.close();

    alfabet = l_system.get_alphabet();

    for (auto it = alfabet.begin(); it != alfabet.end(); it++) {
        // we maken 2 mappen: de drawMap bevat telkens de letter met als value of die getekend moet worden of niet (bool)
        // de replacements bevat de string waarin het symbool moet veranderd worden
        char letter = *it;
        drawMap[letter] = l_system.draw(*it);
        replacements[letter] = l_system.get_replacement(letter);
    }

    angleDegrees = l_system.get_angle(); // hoek in graden
    angle = angleDegrees*(M_PI/180); // hoek in radialen
    initiator = l_system.get_initiator();
    nr_iterations = l_system.get_nr_iterations();

    string code = initiator;
    string code1;

    for (auto i = 0; i < nr_iterations;i++) {
        // hier vervangen we elk symbool in onze huidige string door de replacement rule die ermee
        // overeenkomt (als er meerdere repl. rules zijn ieder met een percentage, dan kiezen we een getal tussen 0 en 1
        // en kijken we in welk interval uit onze map grenzen dit getal zit, zo kiezen we dan de juiste replacement string)
        string tempstring;
        code1 = "";

        for (auto j = 0; j < code.size();j++) {
            if (count(alfabet.begin(), alfabet.end(), code[j])) {
                tempstring = replacements[code[j]][0].first; // origineel zonder stoch. repl. rules
            }
            else {
                tempstring = code[j];
            }
            code1 += tempstring;
        }
        code = code1;
    }

//    list<Line2D> Lines2D;

//    Point2D punt1; punt1.x = 0; punt1.y = 0; // eerste punt start op (0,0)
//    Point2D punt2;

    Figure figuur;

    // huidige locatie
    Vector3D huidigeLocatie = Vector3D::point(0,0,0);
//    huidigeLocatie.x = 0;
//    huidigeLocatie.y = 0;
//    huidigeLocatie.z = 0;

    Vector3D nieuweLocatie;

    // initiële waarden voor de vectoren H, L en U.
    Vector3D H = Vector3D::vector(1,0,0);
//    H.x = 1;
//    H.y = 0;
//    H.z = 0;
    Vector3D L = Vector3D::vector(0,1,0);
//    L.x = 0;
//    L.y = 1;
//    L.z = 0;
    Vector3D U = Vector3D::vector(0,0,1);
//    U.x = 0;
//    U.y = 0;
//    U.z = 1;

//    ambientReflection kleur;
//    kleur.red = ambientReflection[0]; // niet *255, want dit doen we in de functie die we aanroepen om de lijnen te tekenen
//    kleur.blue = ambientReflection[2];
//    kleur.green = ambientReflection[1];

    stack<vector<Vector3D>> stack1;
    int index = 0;
    for (auto c = 0; c < code.size();c++) {

        if (code[c] == '+') { // we draaien naar links over een hoek delta (we roteren de H- en de L-vector delta radialen om de U-vector.
            Vector3D H1 = H*cos(angle) + L*sin(angle);
            Vector3D L1 = -H*sin(angle) + L*cos(angle);
            H = H1;
            L = L1;
        }
        else if (code[c] == '-') { // we draaien naar rechts over een hoek delta.
            Vector3D H1 = H*cos(-angle) + L*sin(-angle);
            Vector3D L1 = -H*sin(-angle) + L*cos(-angle);
            H = H1;
            L = L1;
        }
        else if (code[c] == '^') { // we draaien delta radialen opwaarts
            Vector3D H1 = H*cos(angle) + U*sin(angle);
            Vector3D U1 = -H*sin(angle) + U*cos(angle);
            H = H1;
            U = U1;
        }
        else if (code[c] == '&') { // we draaien delta radialen neerwaarts
            Vector3D H1 = H*cos(-angle) + U*sin(-angle);
            Vector3D U1 = -H*sin(-angle) + U*cos(-angle);
            H = H1;
            U = U1;
        }
        else if (code[c] == '\\') { // we maken een rolbeweging naar links over delta radialen
            Vector3D L1 = L*cos(angle) + U*sin(-angle);
            Vector3D U1 = -L*sin(-angle) + U*cos(angle);
            L = L1;
            U = U1;
        }
        else if (code[c] == '/') { // we maken een rolbeweging naar rechts over delta radialen
            Vector3D L1 = L*cos(angle) + U*sin(angle);
            Vector3D U1 = -L*sin(angle) + U*cos(angle);
            L = L1;
            U = U1;
        }
        else if (code[c] == '|') { // we keren onze richting om
            H = -H;
            L = -L;
        }
        else if (code[c] == '(') {
            vector<Vector3D> vector;
            vector.push_back(huidigeLocatie);
            vector.push_back(H);
            vector.push_back(L);
            vector.push_back(U);

            stack1.push(vector);
        }
        else if (code[c] == ')') {
            huidigeLocatie = stack1.top()[0];
            H = stack1.top()[1];
            L = stack1.top()[2];
            U = stack1.top()[3];

            stack1.pop();
        }
        else {
//            nieuweLocatie.x = huidigeLocatie.x+H.x; // nieuwe x = oude x * cos(hoek)
//            nieuweLocatie.y = huidigeLocatie.y+H.y; // nieuwe y = oude y * sin(hoek)
//            nieuweLocatie.z = huidigeLocatie.z+H.z;
            Vector3D nieuwPunt = huidigeLocatie;
            huidigeLocatie += H;
            if (drawMap[code[c]]) { // als de letter getekend moet worden (bool = true)
//                Line2D lijn = Line2D(punt1, punt2, kleur);
//                Lines2D.push_back(lijn); // we voegen de lijn toe aan de vector Lines2D

                figuur.points.push_back(nieuwPunt);
                figuur.points.push_back(huidigeLocatie);

                Face vlak;
                vlak.point_indexes.push_back(figuur.points.size()-2);
                vlak.point_indexes.push_back(figuur.points.size()-1);

                figuur.faces.push_back(vlak);

                index++;

            }
//            punt1.x = punt2.x; // oud punt wordt nieuw punt
//            punt1.y = punt2.y;
        }
    }
    return figuur;
}

vector<Face> L3Dsysteem::triangulate(const Face& face) {
    vector<Face> vlakken;

//    if (face.point_indexes.size() == 3) {
//        vlakken.push_back(face);
//    }
//    else {
        for (auto i = 0; i < face.point_indexes.size()-2; i++) {
            Face driehoek;
            driehoek.point_indexes.push_back(face.point_indexes[0]);
            driehoek.point_indexes.push_back(face.point_indexes[i+1]);
            driehoek.point_indexes.push_back(face.point_indexes[i+2]);

            vlakken.push_back(driehoek);
        }
//    }
    return vlakken;
}

void L3Dsysteem::generateFractal(Figure &fig, Figures3D &fractal, const int nr_iterations1, const double scale) {


    //fractal.push_back(fig);

    for (auto i = 0; i < nr_iterations1; i++) {
        Figures3D tempFractal;
        for (auto figure = fractal.begin(); figure != fractal.end(); figure++) {
            for (auto j = 0; j < figure->points.size(); j++) {
                Figure figuur;
                figuur.faces = figure->faces;
                figuur.ambientReflection = figure->ambientReflection;
                figuur.diffuseReflection = figure->diffuseReflection;
                figuur.specularReflection = figure->specularReflection;
                figuur.reflectionCoefficient = figure->reflectionCoefficient;
                figuur.points = figure->points;

                Matrix TransformatieMatrix = scaleFigure(1/scale);
                for (auto k = 0; k < figuur.points.size(); k++) {
                    figuur.points[k]*=TransformatieMatrix;
                }
                Vector3D nieuwPunt = figure->points[j]*TransformatieMatrix;
                Vector3D translatieVector = figure->points[j] - nieuwPunt;
                Matrix TransformatieMatrix1 = translate(translatieVector);

                for (auto k = 0; k < figuur.points.size(); k++) {
                    figuur.points[k]*=TransformatieMatrix1;
                }
                //figuur.points.push_back(nieuwPunt*TransformatieMatrix1);

                tempFractal.push_back(figuur);
            }
        }
        fractal = tempFractal;
    }
}

void L3Dsysteem::generateMengerSponge(Figures3D &fractal, const int nr_iterations1, const double scale) {

    for (auto i = 0; i < nr_iterations1; i++) {
        Figures3D tempFractal;
        for (auto figure = fractal.begin(); figure != fractal.end(); figure++) {
            for (auto j = 0; j < figure->points.size(); j++) {
                Figure figuur;
                figuur.faces = figure->faces;
                figuur.ambientReflection = figure->ambientReflection;
                figuur.diffuseReflection = figure->diffuseReflection;
                figuur.specularReflection = figure->specularReflection;
                figuur.reflectionCoefficient = figure->reflectionCoefficient;
                figuur.points = figure->points;

                Matrix TransformatieMatrix = scaleFigure(1/scale);
                for (auto k = 0; k < figuur.points.size(); k++) {
                    figuur.points[k]*=TransformatieMatrix;
                }
                Vector3D nieuwPunt = figure->points[j]*TransformatieMatrix;
                Vector3D translatieVector = figure->points[j] - nieuwPunt;
                Matrix TransformatieMatrix1 = translate(translatieVector);

                for (auto k = 0; k < figuur.points.size(); k++) {
                    figuur.points[k]*=TransformatieMatrix1;
                }

                tempFractal.push_back(figuur);
            }
            Figure middelkubus0;
            middelkubus0.faces = figure->faces;
            middelkubus0.ambientReflection = figure->ambientReflection;
            middelkubus0.diffuseReflection = figure->diffuseReflection;
            middelkubus0.specularReflection = figure->specularReflection;
            middelkubus0.reflectionCoefficient = figure->reflectionCoefficient;
            middelkubus0.points = figure->points;

            Matrix TransformatieMatrix = scaleFigure(1/scale);
            for (auto k = 0; k < middelkubus0.points.size(); k++) {
                middelkubus0.points[k]*=TransformatieMatrix;
            }
            Vector3D oudeMiddelPunt = figure->points[6]+(figure->points[2]-figure->points[6])/2;

            Vector3D nieuwPunt = oudeMiddelPunt*TransformatieMatrix; // = nieuweMiddelPunt
            Vector3D translatieVector = oudeMiddelPunt - nieuwPunt;
            Matrix TransformatieMatrix1 = translate(translatieVector);

            for (auto k = 0; k < middelkubus0.points.size(); k++) {
                middelkubus0.points[k]*=TransformatieMatrix1;
            }
            tempFractal.push_back(middelkubus0);

            Figure middelkubus1;
            middelkubus1.faces = figure->faces;
            middelkubus1.ambientReflection = figure->ambientReflection;
            middelkubus1.diffuseReflection = figure->diffuseReflection;
            middelkubus1.specularReflection = figure->specularReflection;
            middelkubus1.reflectionCoefficient = figure->reflectionCoefficient;
            middelkubus1.points = figure->points;

            TransformatieMatrix = scaleFigure(1/scale);
            for (auto k = 0; k < middelkubus1.points.size(); k++) {
                middelkubus1.points[k]*=TransformatieMatrix;
            }
            oudeMiddelPunt = figure->points[6]+(figure->points[0]-figure->points[6])/2;

            nieuwPunt = oudeMiddelPunt*TransformatieMatrix; // = nieuweMiddelPunt
            translatieVector = oudeMiddelPunt - nieuwPunt;
            TransformatieMatrix1 = translate(translatieVector);

            for (auto k = 0; k < middelkubus1.points.size(); k++) {
                middelkubus1.points[k]*=TransformatieMatrix1;
            }
            tempFractal.push_back(middelkubus1);

            Figure middelkubus2;
            middelkubus2.faces = figure->faces;
            middelkubus2.ambientReflection = figure->ambientReflection;
            middelkubus2.diffuseReflection = figure->diffuseReflection;
            middelkubus2.specularReflection = figure->specularReflection;
            middelkubus2.reflectionCoefficient = figure->reflectionCoefficient;
            middelkubus2.points = figure->points;

            TransformatieMatrix = scaleFigure(1/scale);
            for (auto k = 0; k < middelkubus2.points.size(); k++) {
                middelkubus2.points[k]*=TransformatieMatrix;
            }
            oudeMiddelPunt = figure->points[0]+(figure->points[4]-figure->points[0])/2;

            nieuwPunt = oudeMiddelPunt*TransformatieMatrix; // = nieuweMiddelPunt
            translatieVector = oudeMiddelPunt - nieuwPunt;
            TransformatieMatrix1 = translate(translatieVector);

            for (auto k = 0; k < middelkubus2.points.size(); k++) {
                middelkubus2.points[k]*=TransformatieMatrix1;
            }
            tempFractal.push_back(middelkubus2);

            Figure middelkubus3;
            middelkubus3.faces = figure->faces;
            middelkubus3.ambientReflection = figure->ambientReflection;
            middelkubus3.diffuseReflection = figure->diffuseReflection;
            middelkubus3.specularReflection = figure->specularReflection;
            middelkubus3.reflectionCoefficient = figure->reflectionCoefficient;
            middelkubus3.points = figure->points;

            TransformatieMatrix = scaleFigure(1/scale);
            for (auto k = 0; k < middelkubus3.points.size(); k++) {
                middelkubus3.points[k]*=TransformatieMatrix;
            }
            oudeMiddelPunt = figure->points[4]+(figure->points[2]-figure->points[4])/2;

            nieuwPunt = oudeMiddelPunt*TransformatieMatrix; // = nieuweMiddelPunt
            translatieVector = oudeMiddelPunt - nieuwPunt;
            TransformatieMatrix1 = translate(translatieVector);

            for (auto k = 0; k < middelkubus3.points.size(); k++) {
                middelkubus3.points[k]*=TransformatieMatrix1;
            }
            tempFractal.push_back(middelkubus3);

            Figure middelkubus4;
            middelkubus4.faces = figure->faces;
            middelkubus4.ambientReflection = figure->ambientReflection;
            middelkubus4.diffuseReflection = figure->diffuseReflection;
            middelkubus4.specularReflection = figure->specularReflection;
            middelkubus4.reflectionCoefficient = figure->reflectionCoefficient;
            middelkubus4.points = figure->points;

            TransformatieMatrix = scaleFigure(1/scale);
            for (auto k = 0; k < middelkubus4.points.size(); k++) {
                middelkubus4.points[k]*=TransformatieMatrix;
            }
            oudeMiddelPunt = figure->points[7]+(figure->points[3]-figure->points[7])/2;

            nieuwPunt = oudeMiddelPunt*TransformatieMatrix; // = nieuweMiddelPunt
            translatieVector = oudeMiddelPunt - nieuwPunt;
            TransformatieMatrix1 = translate(translatieVector);

            for (auto k = 0; k < middelkubus4.points.size(); k++) {
                middelkubus4.points[k]*=TransformatieMatrix1;
            }
            tempFractal.push_back(middelkubus4);

            Figure middelkubus5;
            middelkubus5.faces = figure->faces;
            middelkubus5.ambientReflection = figure->ambientReflection;
            middelkubus5.diffuseReflection = figure->diffuseReflection;
            middelkubus5.specularReflection = figure->specularReflection;
            middelkubus5.reflectionCoefficient = figure->reflectionCoefficient;
            middelkubus5.points = figure->points;

            TransformatieMatrix = scaleFigure(1/scale);
            for (auto k = 0; k < middelkubus5.points.size(); k++) {
                middelkubus5.points[k]*=TransformatieMatrix;
            }
            oudeMiddelPunt = figure->points[3]+(figure->points[5]-figure->points[3])/2;

            nieuwPunt = oudeMiddelPunt*TransformatieMatrix; // = nieuweMiddelPunt
            translatieVector = oudeMiddelPunt - nieuwPunt;
            TransformatieMatrix1 = translate(translatieVector);

            for (auto k = 0; k < middelkubus5.points.size(); k++) {
                middelkubus5.points[k]*=TransformatieMatrix1;
            }
            tempFractal.push_back(middelkubus5);

            Figure middelkubus6;
            middelkubus6.faces = figure->faces;
            middelkubus6.ambientReflection = figure->ambientReflection;
            middelkubus6.diffuseReflection = figure->diffuseReflection;
            middelkubus6.specularReflection = figure->specularReflection;
            middelkubus6.reflectionCoefficient = figure->reflectionCoefficient;
            middelkubus6.points = figure->points;

            TransformatieMatrix = scaleFigure(1/scale);
            for (auto k = 0; k < middelkubus6.points.size(); k++) {
                middelkubus6.points[k]*=TransformatieMatrix;
            }
            oudeMiddelPunt = figure->points[5]+(figure->points[1]-figure->points[5])/2;

            nieuwPunt = oudeMiddelPunt*TransformatieMatrix; // = nieuweMiddelPunt
            translatieVector = oudeMiddelPunt - nieuwPunt;
            TransformatieMatrix1 = translate(translatieVector);

            for (auto k = 0; k < middelkubus6.points.size(); k++) {
                middelkubus6.points[k]*=TransformatieMatrix1;
            }
            tempFractal.push_back(middelkubus6);

            Figure middelkubus7;
            middelkubus7.faces = figure->faces;
            middelkubus7.ambientReflection = figure->ambientReflection;
            middelkubus7.diffuseReflection = figure->diffuseReflection;
            middelkubus7.specularReflection = figure->specularReflection;
            middelkubus7.reflectionCoefficient = figure->reflectionCoefficient;
            middelkubus7.points = figure->points;

            TransformatieMatrix = scaleFigure(1/scale);
            for (auto k = 0; k < middelkubus7.points.size(); k++) {
                middelkubus7.points[k]*=TransformatieMatrix;
            }
            oudeMiddelPunt = figure->points[1]+(figure->points[7]-figure->points[1])/2;

            nieuwPunt = oudeMiddelPunt*TransformatieMatrix; // = nieuweMiddelPunt
            translatieVector = oudeMiddelPunt - nieuwPunt;
            TransformatieMatrix1 = translate(translatieVector);

            for (auto k = 0; k < middelkubus7.points.size(); k++) {
                middelkubus7.points[k]*=TransformatieMatrix1;
            }
            tempFractal.push_back(middelkubus7);

            Figure middelkubus8;
            middelkubus8.faces = figure->faces;
            middelkubus8.ambientReflection = figure->ambientReflection;
            middelkubus8.diffuseReflection = figure->diffuseReflection;
            middelkubus8.specularReflection = figure->specularReflection;
            middelkubus8.reflectionCoefficient = figure->reflectionCoefficient;
            middelkubus8.points = figure->points;

            TransformatieMatrix = scaleFigure(1/scale);
            for (auto k = 0; k < middelkubus8.points.size(); k++) {
                middelkubus8.points[k]*=TransformatieMatrix;
            }
            oudeMiddelPunt = figure->points[3]+(figure->points[6]-figure->points[3])/2;

            nieuwPunt = oudeMiddelPunt*TransformatieMatrix; // = nieuweMiddelPunt
            translatieVector = oudeMiddelPunt - nieuwPunt;
            TransformatieMatrix1 = translate(translatieVector);

            for (auto k = 0; k < middelkubus8.points.size(); k++) {
                middelkubus8.points[k]*=TransformatieMatrix1;
            }
            tempFractal.push_back(middelkubus8);

            Figure middelkubus9;
            middelkubus9.faces = figure->faces;
            middelkubus9.ambientReflection = figure->ambientReflection;
            middelkubus9.diffuseReflection = figure->diffuseReflection;
            middelkubus9.specularReflection = figure->specularReflection;
            middelkubus9.reflectionCoefficient = figure->reflectionCoefficient;
            middelkubus9.points = figure->points;

            TransformatieMatrix = scaleFigure(1/scale);
            for (auto k = 0; k < middelkubus9.points.size(); k++) {
                middelkubus9.points[k]*=TransformatieMatrix;
            }
            oudeMiddelPunt = figure->points[5]+(figure->points[0]-figure->points[5])/2;

            nieuwPunt = oudeMiddelPunt*TransformatieMatrix; // = nieuweMiddelPunt
            translatieVector = oudeMiddelPunt - nieuwPunt;
            TransformatieMatrix1 = translate(translatieVector);

            for (auto k = 0; k < middelkubus9.points.size(); k++) {
                middelkubus9.points[k]*=TransformatieMatrix1;
            }
            tempFractal.push_back(middelkubus9);

            Figure middelkubus10;
            middelkubus10.faces = figure->faces;
            middelkubus10.ambientReflection = figure->ambientReflection;
            middelkubus10.diffuseReflection = figure->diffuseReflection;
            middelkubus10.specularReflection = figure->specularReflection;
            middelkubus10.reflectionCoefficient = figure->reflectionCoefficient;
            middelkubus10.points = figure->points;

            TransformatieMatrix = scaleFigure(1/scale);
            for (auto k = 0; k < middelkubus10.points.size(); k++) {
                middelkubus10.points[k]*=TransformatieMatrix;
            }
            oudeMiddelPunt = figure->points[4]+(figure->points[1]-figure->points[4])/2;

            nieuwPunt = oudeMiddelPunt*TransformatieMatrix; // = nieuweMiddelPunt
            translatieVector = oudeMiddelPunt - nieuwPunt;
            TransformatieMatrix1 = translate(translatieVector);

            for (auto k = 0; k < middelkubus10.points.size(); k++) {
                middelkubus10.points[k]*=TransformatieMatrix1;
            }
            tempFractal.push_back(middelkubus10);

            Figure middelkubus11;
            middelkubus11.faces = figure->faces;
            middelkubus11.ambientReflection = figure->ambientReflection;
            middelkubus11.diffuseReflection = figure->diffuseReflection;
            middelkubus11.specularReflection = figure->specularReflection;
            middelkubus11.reflectionCoefficient = figure->reflectionCoefficient;
            middelkubus11.points = figure->points;

            TransformatieMatrix = scaleFigure(1/scale);
            for (auto k = 0; k < middelkubus11.points.size(); k++) {
                middelkubus11.points[k]*=TransformatieMatrix;
            }
            oudeMiddelPunt = figure->points[7]+(figure->points[2]-figure->points[7])/2;

            nieuwPunt = oudeMiddelPunt*TransformatieMatrix; // = nieuweMiddelPunt
            translatieVector = oudeMiddelPunt - nieuwPunt;
            TransformatieMatrix1 = translate(translatieVector);

            for (auto k = 0; k < middelkubus11.points.size(); k++) {
                middelkubus11.points[k]*=TransformatieMatrix1;
            }
            tempFractal.push_back(middelkubus11);

        }
        fractal = tempFractal;
    }
}

void L3Dsysteem::calculateShadowMask(const int size, list<Figure> &figuren, ZBuffer &shadowMask, Light& licht) {
    double posInf = numeric_limits<double>::infinity();
    double negInf = -numeric_limits<double>::infinity();

    double Xmax = negInf;
    double Xmin = posInf;
    double Ymax = negInf;
    double Ymin = posInf;

    for (auto it = figuren.begin(); it != figuren.end(); it++) {
        for (auto i = 0; i < it->points.size(); i++) {
            Point2D punt;
            double d = 1;
            punt.x = (d * it->points[i].x) / (-it->points[i].z);
            punt.y = (d * it->points[i].y) / (-it->points[i].z);

            if (punt.x >
                Xmax) { // als één van de x coordinaten van de nieuwe lijn groter is, wordt dat het nieuwe maximum
                Xmax = punt.x;
            }
            if (punt.x <
                Xmin) { // als één van de x coordinaten van de nieuwe lijn kleiner is, wordt dat het nieuwe minimum
                Xmin = punt.x;
            }
            if (punt.y >
                Ymax) { // als één van de y coordinaten van de nieuwe lijn groter is, wordt dat het nieuwe maximum
                Ymax = punt.y;
            }
            if (punt.y <
                Ymin) { // als één van de y coordinaten van de nieuwe lijn groter is, wordt dat het nieuwe maximum
                Ymin = punt.y;
            }
        }
    }

    double Xrange = Xmax - Xmin;
    double Yrange = Ymax - Ymin;

    double ImageX = size * (Xrange / std::max(Xrange, Yrange));
    double ImageY = size * (Yrange / std::max(Xrange, Yrange));

    licht.shadowMask1 = ZBuffer(ImageX, ImageY);
    //img::EasyImage image(roundToInt(ImageX), roundToInt(ImageY));

    double d = 0.95 * (ImageX / Xrange); // = d

    licht.d = d;

    double DCx = d * ((Xmin + Xmax) / 2);
    double DCy = d * ((Ymin + Ymax) / 2);
    double dx = ImageX / 2 - DCx;
    double dy = ImageY / 2 - DCy;
    licht.dx = dx;
    licht.dy = dy;

    for (auto &it : figuren) {

        for (auto it2 = 0; it2 < it.faces.size(); it2++) {
            Vector3D A = Vector3D::point(it.points[it.faces[it2].point_indexes[0]].x,
                                         it.points[it.faces[it2].point_indexes[0]].y,
                                         it.points[it.faces[it2].point_indexes[0]].z); // eerste punt van de driehoek

            Vector3D B = Vector3D::point(it.points[it.faces[it2].point_indexes[1]].x,
                                         it.points[it.faces[it2].point_indexes[1]].y,
                                         it.points[it.faces[it2].point_indexes[1]].z); // tweede punt van de driehoek

            Vector3D C = Vector3D::point(it.points[it.faces[it2].point_indexes[2]].x,
                                         it.points[it.faces[it2].point_indexes[2]].y,
                                         it.points[it.faces[it2].point_indexes[2]].z); // derde punt van de driehoek

            Point2D _A;
            _A.x = (licht.d * A.x / -A.z) + licht.dx;
            _A.y = (licht.d * A.y / -A.z) + licht.dy;

            Point2D _B;
            _B.x = (licht.d * B.x / -B.z) + licht.dx;
            _B.y = (licht.d * B.y / -B.z) + licht.dy;

            Point2D _C;
            _C.x = (licht.d * C.x / -C.z) + licht.dx;
            _C.y = (licht.d * C.y / -C.z) + licht.dy;

            vector<Point2D> puntenDriehoek; // vector die de geprojecteerde punten bevat
            puntenDriehoek.push_back(_A);
            puntenDriehoek.push_back(_B);
            puntenDriehoek.push_back(_C);

            //bepalen Ymin en Ymax

            int Ymin = roundToInt(min({_A.y, _B.y, _C.y}) +
                                  0.5); // https://stackoverflow.com/questions/9424173/find-the-smallest-amongst-3-numbers-in-c
            int Ymax = roundToInt(max({_A.y, _B.y, _C.y}) - 0.5);

            double posInf = numeric_limits<double>::infinity();
            double negInf = -numeric_limits<double>::infinity();

            //zwaartepunt
            Point2D zwaartepunt;
            zwaartepunt.x = (_A.x + _B.x + _C.x) / 3;
            zwaartepunt.y = (_A.y + _B.y + _C.y) / 3;
            double _1opZ_G = (1 / (3 * A.z)) + (1 / (3 * B.z)) + (1 / (3 * C.z)); // 1 over Z waarde van zwaartepunt

            // berekening dzdx en dzdy
            Vector3D u = Vector3D::vector(B - A); ///
            Vector3D v = Vector3D::vector(C - A);
            Vector3D w = Vector3D::vector(Vector3D::cross(u, v));
            double k = (w.x * A.x) + (w.y * A.y) + (w.z * A.z); // rond cursus 45
            double dzdx = w.x / (-licht.d * k);
            double dzdy = w.y / (-licht.d * k);

            double xI;
            for (int i = Ymin; i <= Ymax; i++) { // elke y waarde doorlopen

                double xL_AB = posInf;
                double xL_AC = posInf;
                double xL_BC = posInf;
                double xR_AB = negInf;
                double xR_AC = negInf;
                double xR_BC = negInf;

                if ((i - _A.y) * (i - _C.y) <= 0 && _A.y != _C.y) { // AC
                    xI = _C.x + ((_A.x - _C.x) * ((i - _C.y) / (_A.y -
                                                                _C.y))); // deze manier van het bepalen van de xI was een tip van een medeleerling (Said).
                    // Ik bedoel de onderverdeling in drie if statements. Oorspronkelijk had ik hier, net zoals in de cursus, een for loop staan die loopte over de zijden van de driehoek,
                    // maar deze manier is efficiënter en overzichtelijker.
                    xL_AC = xI;
                    xR_AC = xI;
                }
                if ((i - _A.y) * (i - _B.y) <= 0 && _A.y != _B.y) { // AB
                    xI = _B.x + ((_A.x - _B.x) * ((i - _B.y) / (_A.y - _B.y)));
                    xL_AB = xI;
                    xR_AB = xI;
                }
                if ((i - _B.y) * (i - _C.y) <= 0 && _B.y != _C.y) { // BC
                    xI = _C.x + ((_B.x - _C.x) * ((i - _C.y) / (_B.y - _C.y)));
                    xL_BC = xI;
                    xR_BC = xI;
                }

                int xLmin = roundToInt(min({xL_AB, xL_BC, xL_AC}) + 0.5);
                int xRmax = roundToInt(max({xR_AB, xR_BC, xR_AC}) - 0.5);
                for (int j = xLmin; j <= xRmax; j++) { // elke x waarde doorlopen van xLmin tot xRmax
                    double _1opZ = (_1opZ_G * 1.0001) + ((j - zwaartepunt.x) * dzdx) +
                                   ((i - zwaartepunt.y) * dzdy); // formule dia 12 (ppt) , moet 1.0001 daar staan?

                    if (_1opZ < licht.shadowMask1.pixelArray[j][i]) { // als z waarde kleiner is dan die in de buffer
                        licht.shadowMask1.pixelArray[j][i] = _1opZ;
                    }
                }
            }
        }
    }
}
