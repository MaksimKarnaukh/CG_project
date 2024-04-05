//
// Created by centu on 5/05/2021.
//

#include "drawTriangles.h"

img::EasyImage drawTriangles(const int size, const vector<double>& backgroundColor, list<Figure> &figuren, Lights3D &lichten, bool lighted, vector<double> eye) {

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

            if (punt.x > Xmax) { // als één van de x coordinaten van de nieuwe lijn groter is, wordt dat het nieuwe maximum
                Xmax = punt.x;
            }
            if (punt.x < Xmin) { // als één van de x coordinaten van de nieuwe lijn kleiner is, wordt dat het nieuwe minimum
                Xmin = punt.x;
            }
            if (punt.y > Ymax) { // als één van de y coordinaten van de nieuwe lijn groter is, wordt dat het nieuwe maximum
                Ymax = punt.y;
            }
            if (punt.y < Ymin) { // als één van de y coordinaten van de nieuwe lijn groter is, wordt dat het nieuwe maximum
                Ymin = punt.y;
            }
        }
    }

    double Xrange = Xmax - Xmin;
    double Yrange = Ymax - Ymin;

    double ImageX = size*(Xrange/std::max(Xrange, Yrange));
    double ImageY = size*(Yrange/std::max(Xrange, Yrange));

    ZBuffer zBuffer(ImageX, ImageY);

    img::EasyImage image(roundToInt(ImageX), roundToInt(ImageY));
    img::Color achtergrond;
    achtergrond.red = backgroundColor[0]*255;
    achtergrond.blue = backgroundColor[2]*255;
    achtergrond.green = backgroundColor[1]*255;
    image.clear(achtergrond); // achtergrondkleur zetten

    double schaalfactor = 0.95*(ImageX/Xrange); // = d

    double DCx = schaalfactor*((Xmin+Xmax)/2);
    double DCy = schaalfactor*((Ymin+Ymax)/2);
    double dx = ImageX/2 - DCx;
    double dy = ImageY/2 - DCy;

    for (auto &it : figuren) {

        Color ambientRefl;
        ambientRefl.red = it.ambientReflection.red;
        ambientRefl.green = it.ambientReflection.green;
        ambientRefl.blue = it.ambientReflection.blue;

        Color diffuse;
        diffuse.red = it.diffuseReflection.red;
        diffuse.blue = it.diffuseReflection.blue;
        diffuse.green = it.diffuseReflection.green;

        Color specular;
        specular.red = it.specularReflection.red;
        specular.blue = it.specularReflection.blue;
        specular.green = it.specularReflection.green;

        double reflectionC = it.reflectionCoefficient;

        for (auto it2 = 0; it2 < it.faces.size(); it2++) {
            Vector3D punt1 = Vector3D::point(it.points[it.faces[it2].point_indexes[0]].x,
                                             it.points[it.faces[it2].point_indexes[0]].y,
                                             it.points[it.faces[it2].point_indexes[0]].z); // eerste punt van de driehoek
//            punt1.x = it.points[it.faces[it2].point_indexes[0]].x;
//            punt1.y = it.points[it.faces[it2].point_indexes[0]].y;
//            punt1.z = it.points[it.faces[it2].point_indexes[0]].z;

            Vector3D punt2 = Vector3D::point(it.points[it.faces[it2].point_indexes[1]].x,
                                             it.points[it.faces[it2].point_indexes[1]].y,
                                             it.points[it.faces[it2].point_indexes[1]].z); // tweede punt van de driehoek
//            punt2.x = it.points[it.faces[it2].point_indexes[1]].x;
//            punt2.y = it.points[it.faces[it2].point_indexes[1]].y;
//            punt2.z = it.points[it.faces[it2].point_indexes[1]].z;

            Vector3D punt3 = Vector3D::point(it.points[it.faces[it2].point_indexes[2]].x,
                                             it.points[it.faces[it2].point_indexes[2]].y,
                                             it.points[it.faces[it2].point_indexes[2]].z); // derde punt van de driehoek
//            punt3.x = it.points[it.faces[it2].point_indexes[2]].x;
//            punt3.y = it.points[it.faces[it2].point_indexes[2]].y;
//            punt3.z = it.points[it.faces[it2].point_indexes[2]].z;

            draw_zbuf_triag(image, zBuffer, punt1, punt2, punt3, schaalfactor, dx, dy, ambientRefl, diffuse, specular, reflectionC, lichten, lighted, eye);
        }
    }
    return image;
}

void draw_zbuf_triag(img::EasyImage &image, ZBuffer &zbuffer, const Vector3D &A, const Vector3D &B, const Vector3D &C, double d,
                                     double dx, double dy, Color &ambientReflection, Color &diffuseReflection, Color specularReflection, double reflectionCoeff, Lights3D &lights, bool lighted, vector<double> eye) {

    Point2D _A;
    _A.x = (d*A.x/-A.z)+dx;
    _A.y = (d*A.y/-A.z)+dy;

    Point2D _B;
    _B.x = (d*B.x/-B.z)+dx;
    _B.y = (d*B.y/-B.z)+dy;

    Point2D _C;
    _C.x = (d*C.x/-C.z)+dx;
    _C.y = (d*C.y/-C.z)+dy;

    vector<Point2D> puntenDriehoek; // vector die de geprojecteerde punten bevat
    puntenDriehoek.push_back(_A);
    puntenDriehoek.push_back(_B);
    puntenDriehoek.push_back(_C);

    //bepalen Ymin en Ymax

    int Ymin = roundToInt(min({_A.y, _B.y, _C.y})+0.5); // https://stackoverflow.com/questions/9424173/find-the-smallest-amongst-3-numbers-in-c
    int Ymax = roundToInt(max({_A.y, _B.y, _C.y})-0.5);

    double posInf = numeric_limits<double>::infinity();
    double negInf = -numeric_limits<double>::infinity();

    //zwaartepunt
    Point2D zwaartepunt;
    zwaartepunt.x = (_A.x+_B.x+_C.x)/3;
    zwaartepunt.y = (_A.y+_B.y+_C.y)/3;
    double _1opZ_G = (1/(3*A.z))+(1/(3*B.z))+(1/(3*C.z)); // 1 over Z waarde van zwaartepunt

    // berekening dzdx en dzdy
    Vector3D u = Vector3D::vector(B - A); ///
    Vector3D v = Vector3D::vector(C - A);
    Vector3D w = Vector3D::vector(Vector3D::cross(u,v));
    double k = (w.x*A.x) + (w.y*A.y) + (w.z*A.z); // rond cursus 45
    double dzdx = w.x/(-d*k);
    double dzdy = w.y/(-d*k);

    //ambient
    Vector3D n;
    Vector3D l;

    Color pixelKleur;
    Color ambient;

    if (!lighted) {
        pixelKleur.red = ambientReflection.red;
        pixelKleur.green = ambientReflection.green;
        pixelKleur.blue = ambientReflection.blue;

    }
    else {
        n = Vector3D::vector(Vector3D::normalise(w)); // vector n
        vector<double> kleur1{0,0,0};

        vector<double> kleur{0,0,0};
        for (auto licht = lights.begin(); licht != lights.end(); licht++) {

            kleur[0] += ambientReflection.red*licht->ambientLight.red;
            kleur[1] += ambientReflection.green*licht->ambientLight.green;
            kleur[2] += ambientReflection.blue*licht->ambientLight.blue;

            if (licht->infinity) { // als de INF factor true is
                l = Vector3D::vector(-licht->ldVector);
                l = Vector3D::normalise(l);

                Vector3D eyepoint = Vector3D::point(eye[0], eye[1], eye[2]);
                L3Dsysteem a;
                Matrix m = a.eyePointTrans(eyepoint);
                l*=m;
                double cosinus = Vector3D::dot(n, l); //n.x*l.x + n.y*l.y + n.z*l.z;
                if (cosinus > 0) {
//                    if (cosinus > 1) {
//                        cosinus = 1;
//                    }
                    kleur1[0] += diffuseReflection.red*licht->diffuseLight.red*cosinus;
                    kleur1[1] += diffuseReflection.green*licht->diffuseLight.green*cosinus;
                    kleur1[2] += diffuseReflection.blue*licht->diffuseLight.blue*cosinus;
                }
            }
        }
        ambient.red = kleur[0]; // niet maal 255 doen hier
        ambient.green = kleur[1];
        ambient.blue = kleur[2];

        Color diffuse;
        diffuse.red = kleur1[0];
        diffuse.green = kleur1[1];
        diffuse.blue = kleur1[2];

        pixelKleur.red = ambient.red + diffuse.red;
        pixelKleur.green = ambient.green + diffuse.green;
        pixelKleur.blue = ambient.blue + diffuse.blue;

        if (pixelKleur.red > 1) {
            pixelKleur.red = 1;
        }
        if (pixelKleur.green > 1) {
            pixelKleur.green = 1;
        }
        if (pixelKleur.blue > 1) {
            pixelKleur.blue = 1;
        }
    }

    double xI;
    for (int i = Ymin; i <= Ymax; i++) { // elke y waarde doorlopen

        double xL_AB = posInf; double xL_AC = posInf; double xL_BC = posInf;
        double xR_AB = negInf; double xR_AC = negInf; double xR_BC = negInf;

        if ((i-_A.y)*(i-_C.y) <= 0 && _A.y != _C.y) { // AC
            xI = _C.x + ((_A.x-_C.x)*((i-_C.y)/(_A.y-_C.y))); // deze manier van het bepalen van de xI was een tip van een medeleerling (Said).
            // Ik bedoel de onderverdeling in drie if statements. Oorspronkelijk had ik hier, net zoals in de cursus, een for loop staan die loopte over de zijden van de driehoek,
            // maar deze manier is efficiënter en overzichtelijker.
            xL_AC = xI;
            xR_AC = xI;
        }
        if ((i-_A.y)*(i-_B.y) <= 0 && _A.y != _B.y) { // AB
            xI = _B.x + ((_A.x-_B.x)*((i-_B.y)/(_A.y-_B.y)));
            xL_AB = xI;
            xR_AB = xI;
        }
        if ((i-_B.y)*(i-_C.y) <= 0 && _B.y != _C.y) { // BC
            xI = _C.x + ((_B.x-_C.x)*((i-_C.y)/(_B.y-_C.y)));
            xL_BC = xI;
            xR_BC = xI;
        }

        int xLmin = roundToInt(min({xL_AB, xL_BC, xL_AC})+0.5);
        int xRmax = roundToInt(max({xR_AB, xR_BC, xR_AC})-0.5);
        for (int j = xLmin; j <= xRmax; j++) { // elke x waarde doorlopen van xLmin tot xRmax
            bool draw = false;
            double _1opZ = (_1opZ_G*1.0001) + ((j-zwaartepunt.x)*dzdx) + ((i-zwaartepunt.y)*dzdy); // formule dia 12 (ppt) , moet 1.0001 daar staan?

            if (_1opZ < zbuffer.pixelArray[j][i]) { // als z waarde kleiner is dan die in de buffer

                double zWaarde = 1/_1opZ;
                Color tempPixelKleur = pixelKleur;

                if (lighted) {
                    Vector3D punt = Vector3D::point(((double)j-dx)*(-zWaarde)/d, ((double)i-dy)*(-zWaarde)/d, zWaarde);
                    vector<double> kleur3{0,0,0};
                    for (auto& licht:lights) { // by reference werkt? ///

                        double cosinus;
                        //Vector3D l;

                        if (licht.infinity == false) { // als de INF factor true is (=puntlicht)
                            if (licht.isShadow) {
                                Vector3D Pe = punt;
                                Vector3D eyepoint = Vector3D::vector(eye[0], eye[1], eye[2]);
                                L3Dsysteem a;
                                Matrix m = a.eyePointTrans(eyepoint);
                                Matrix inv = Matrix::inv(m);

                                Pe *= inv;
                                Pe *= licht.eye; // maal Vl

                                double xL = (licht.d*Pe.x)/(-Pe.z) + licht.dx;
                                double yL = (licht.d*Pe.y)/(-Pe.z) + licht.dy;

                                double Ax = xL - floor(xL);
                                double Ay = yL - floor(yL);

                                Point2D Za;
                                Za.x = floor(xL);
                                Za.y = ceil(yL);
                                Point2D Zb;
                                Zb.x = ceil(xL);
                                Zb.y = ceil(yL);
                                Point2D Zc;
                                Zc.x = floor(xL);
                                Zc.y = floor(yL);
                                Point2D Zd;
                                Zd.x = ceil(xL);
                                Zd.y = floor(yL);

                                double ZA = licht.shadowMask1.pixelArray[Za.x][Za.y];
                                double ZB = licht.shadowMask1.pixelArray[Zb.x][Zb.y];
                                double ZC = licht.shadowMask1.pixelArray[Zc.x][Zc.y];
                                double ZD = licht.shadowMask1.pixelArray[Zd.x][Zd.y];

                                double _1opZe = (1-Ax)*(ZA) + Ax*(ZB);
                                double _1opZf = (1-Ax)*(ZC) + Ax*(ZD);

                                double _1opZl = Ay*(_1opZe) + (1-Ay)*(_1opZf);

                                if (1/Pe.z < _1opZl || abs(1/Pe.z - _1opZl) < 0.000005) {
                                    draw = true;
                                }
                            }
                            if (licht.isShadow == false || (licht.isShadow == true && draw == true)) {
                                Vector3D eyepoint = Vector3D::point(eye[0], eye[1], eye[2]);
                                L3Dsysteem a;
                                Matrix m = a.eyePointTrans(eyepoint);
                                Vector3D location = Vector3D::point(licht.location);
                                location*=m;

                                l = Vector3D::vector(location-punt);
                                l = Vector3D::normalise(l);

                                cosinus = Vector3D::dot(n, l); //n.x*l.x + n.y*l.y + n.z*l.z;
                                if (licht.isSpotAngle) {
                                    if (cosinus > 0 && cosinus > cos(licht.spotAngle*M_PI/180)) {

                                        kleur3[0] += diffuseReflection.red*licht.diffuseLight.red*(1-((1-cosinus)/(1-cos(licht.spotAngle*M_PI/180))));
                                        kleur3[1] += diffuseReflection.green*licht.diffuseLight.green*(1-((1-cosinus)/(1-cos(licht.spotAngle*M_PI/180))));
                                        kleur3[2] += diffuseReflection.blue*licht.diffuseLight.blue*(1-((1-cosinus)/(1-cos(licht.spotAngle*M_PI/180))));
                                    }
                                }
                                else {
                                    if (cosinus > 0) {

                                        kleur3[0] += diffuseReflection.red*licht.diffuseLight.red*cosinus;
                                        kleur3[1] += diffuseReflection.green*licht.diffuseLight.green*cosinus;
                                        kleur3[2] += diffuseReflection.blue*licht.diffuseLight.blue*cosinus;
                                    }
                                }
                            }
                        }
                        if (licht.isShadow == false || (licht.isShadow == true && draw == true)) {
                            if (licht.isSpecular) {

                                Vector3D r = Vector3D::vector(2*cosinus*n-l);
                                Vector3D punt1 = Vector3D::vector(-punt);
                                punt1 = Vector3D::normalise(punt1);
                                double cosinusB = Vector3D::dot(r, punt1);

                                if (cosinusB > 0) {
                                    kleur3[0] += specularReflection.red*licht.specularLight.red*pow(cosinusB, reflectionCoeff); // cosB^ms
                                    kleur3[1] += specularReflection.green*licht.specularLight.green*pow(cosinusB, reflectionCoeff);
                                    kleur3[2] += specularReflection.blue*licht.specularLight.blue*pow(cosinusB, reflectionCoeff);
                                }
                            }
                        }
                        else if (licht.isShadow == true && draw == false) {
                            pixelKleur.red = ambient.red;
                            pixelKleur.green = ambient.green;
                            pixelKleur.blue = ambient.blue;
                        }
                    }


                    Color diffusePuntLicht;
                    diffusePuntLicht.red = kleur3[0];
                    diffusePuntLicht.green = kleur3[1];
                    diffusePuntLicht.blue = kleur3[2];

                    if (diffusePuntLicht.red > 1) { // eigenlijk niet nodig, maar ik laat het toch staan, want ik heb geen tijd meer om alles na te kijken
                        diffusePuntLicht.red = 1;
                    }
                    if (diffusePuntLicht.green > 1) {
                        diffusePuntLicht.green = 1;
                    }
                    if (diffusePuntLicht.blue > 1) {
                        diffusePuntLicht.blue = 1;
                    }

                    pixelKleur.red = pixelKleur.red + diffusePuntLicht.red;
                    pixelKleur.green = pixelKleur.green + diffusePuntLicht.green;
                    pixelKleur.blue = pixelKleur.blue + diffusePuntLicht.blue;
                }

                if (pixelKleur.red > 1) {
                    pixelKleur.red = 1;
                }
                if (pixelKleur.green > 1) {
                    pixelKleur.green = 1;
                }
                if (pixelKleur.blue > 1) {
                    pixelKleur.blue = 1;
                }

                pixelKleur.red*=255;
                pixelKleur.green*=255;
                pixelKleur.blue*=255;

                (image)(j, i).red = pixelKleur.red;
                (image)(j, i).green = pixelKleur.green;
                (image)(j, i).blue = pixelKleur.blue;
                zbuffer.pixelArray[j][i] = _1opZ;


                pixelKleur = tempPixelKleur; // deze bug heeft mij 6 uur van mijn tijd gekost, je moet dat dus resetten.
            }
        }
    }
}
