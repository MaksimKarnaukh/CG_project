#include "easy_image.h"
#include "ini_configuration.h"
#include "Lsysteem.h"
#include "L2Dsysteem.h"
#include "L3Dsysteem.h"
#include "ZBuffer.h"
#include "drawTriangles.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <cmath>
#include <vector>
using namespace std;

img::EasyImage generate_2DImage(const ini::Configuration &configuration) {
    // parsen van .ini bestand
    int size = configuration["General"]["size"].as_int_or_die();
    vector<double> backgroundColor = configuration["General"]["backgroundcolor"].as_double_tuple_or_die();
    string inputFile = configuration["2DLSystem"]["inputfile"].as_string_or_die();
    vector<double> color = configuration["2DLSystem"]["color"].as_double_tuple_or_die();

    L2Dsysteem a = L2Dsysteem();
    std::list<Line2D> lines = a.parse(inputFile, color);
    return draw2DLines(lines, size, backgroundColor, false);
}

img::EasyImage generate_3DImage(const ini::Configuration &configuration) {
    //vector<double> color;
    // parsen van .ini bestand
    int size = configuration["General"]["size"].as_int_or_die();
    vector<double> backgroundColor = configuration["General"]["backgroundcolor"].as_double_tuple_or_die();
    int nrFigures = configuration["General"]["nrFigures"].as_int_or_die();
    vector<double> eye = configuration["General"]["eye"].as_double_tuple_or_die();

    L3Dsysteem a = L3Dsysteem();
    std::list<Line2D> lines;

    if (configuration["General"]["type"].as_string_or_die() == "ZBuffering") {
        lines = a.parse(configuration, nrFigures, eye, true, false, 0); // hier hebben we enkel de figuren nodig, niet de lijnen
        Figures3D figuren = a.figuren3D;
        Lights3D lichten = a.lights;
        if (figuren.empty()) {
            return img::EasyImage();
        }
        return drawTriangles(size, backgroundColor, figuren, lichten, false, eye);
    }
    else if (configuration["General"]["type"].as_string_or_die() == "LightedZBuffering") {
        int nrLights = configuration["General"]["nrLights"].as_int_or_die();
        lines = a.parse(configuration, nrFigures, eye, true, true, nrLights); // hier hebben we enkel de figuren nodig, niet de lijnen
        Figures3D figuren = a.figuren3D;
        Lights3D lichten = a.lights;
        if (figuren.empty()) {
            return img::EasyImage();
        }
        return drawTriangles(size, backgroundColor, figuren, lichten, true, eye);
    }
    else {
        lines = a.parse(configuration, nrFigures, eye, false, false, 0);
    }

    if (configuration["General"]["type"].as_string_or_die() == "ZBufferedWireframe") {

        return draw2DLines(lines, size, backgroundColor, true); // zbuf lijnen

    }
    else {
        return draw2DLines(lines, size, backgroundColor, false); // lijnen zonder zbuf
    }

}


img::EasyImage generate_image(const ini::Configuration &configuration)
{
    string type = configuration["General"]["type"].as_string_or_die();

    if (configuration["General"]["clipping"].exists()) {
        if (configuration["General"]["clipping"].as_bool_or_die() == true) {
            return img::EasyImage();
        }
    }

    if (type == "2DLSystem") {
        return generate_2DImage(configuration);
    }
    else if (type == "Wireframe" || type == "ZBufferedWireframe" || type == "ZBuffering" || type == "LightedZBuffering") {
        return generate_3DImage(configuration);
    }
    return img::EasyImage();


// oef 1 lijnen (gecentreerd)
//    std::list<Line2D> lines;
//    Point2D punt1;
//    punt1.x = 0;
//    punt1.y = 100;
//
//    Point2D punt2;
//    punt2.x = 100;
//    punt2.y = 0;
//
//    Color kleur;
//    kleur.red = 0.8;
//    kleur.blue = 0;
//    kleur.green = 0;
//    Line2D lijn1 = Line2D(punt1, punt2, kleur);
//    lines.push_back(lijn1);
//    return draw2DLines(lines, 200);
//    std::list<Line2D> lines;
//    for (auto i = 0; i < 5; i++) {
//        Point2D punt1;
//        punt1.x = i+1;
//        punt1.y = 10;
//
//        Point2D punt2;
//        punt2.x = 10*i+1;
//        punt2.y = 20+10*i;
//
//        Color kleur;
//        kleur.red = 0.8;
//        kleur.blue = 0;
//        kleur.green = 0;
//        Line2D lijn1 = Line2D(punt1, punt2, kleur);
//        lines.push_back(lijn1);
//    }
//    for (auto i = 0; i < 20; i++) {
//        Point2D punt1;
//        punt1.x = 100+10*i;
//        punt1.y = 10;
//
//        Point2D punt2;
//        punt2.x = 100+10*i;
//        punt2.y = 20+10*i;
//
//        Color kleur;
//        kleur.red = 1;
//        kleur.blue = 0;
//        kleur.green = 0;
//        Line2D lijn1 = Line2D(punt1, punt2, kleur);
//        lines.push_back(lijn1);
//    }

//    return draw2DLines(lines, 500);
}



int main(int argc, char const* argv[])
{
        int retVal = 0;
        try
        {
                for(int i = 1; i < argc; ++i)
                {
                        ini::Configuration conf;
                        try
                        {
                                std::ifstream fin(argv[i]);
                                fin >> conf;
                                fin.close();
                        }
                        catch(ini::ParseException& ex)
                        {
                                std::cerr << "Error parsing file: " << argv[i] << ": " << ex.what() << std::endl;
                                retVal = 1;
                                continue;
                        }

                        img::EasyImage image = generate_image(conf);
                        if(image.get_height() > 0 && image.get_width() > 0)
                        {
                                std::string fileName(argv[i]);
                                std::string::size_type pos = fileName.rfind('.');
                                if(pos == std::string::npos)
                                {
                                        //filename does not contain a '.' --> append a '.bmp' suffix
                                        fileName += ".bmp";
                                }
                                else
                                {
                                        fileName = fileName.substr(0,pos) + ".bmp";
                                }
                                try
                                {
                                        std::ofstream f_out(fileName.c_str(),std::ios::trunc | std::ios::out | std::ios::binary);
                                        f_out << image;

                                }
                                catch(std::exception& ex)
                                {
                                        std::cerr << "Failed to write image to file: " << ex.what() << std::endl;
                                        retVal = 1;
                                }
                        }
                        else
                        {
                                std::cout << "Could not generate image for " << argv[i] << std::endl;
                        }
                }
        }
        catch(const std::bad_alloc &exception)
        {
    		//When you run out of memory this exception is thrown. When this happens the return value of the program MUST be '100'.
    		//Basically this return value tells our automated test scripts to run your engine on a pc with more memory.
    		//(Unless of course you are already consuming the maximum allowed amount of memory)
    		//If your engine does NOT adhere to this requirement you risk losing points because then our scripts will
		//mark the test as failed while in reality it just needed a bit more memory
                std::cerr << "Error: insufficient memory" << std::endl;
                retVal = 100;
        }

        return retVal;
}
