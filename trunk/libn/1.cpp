#include <iostream>
#include <noise/noise.h>
#include "noiseutils.h"

using namespace noise;

int main(int argc, char** argv) {
    module::Perlin myModule;
    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderPlane heightMapBuilder;
    heightMapBuilder.SetSourceModule(myModule);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    heightMapBuilder.SetDestSize(256, 256);
    heightMapBuilder.SetBounds(2.0, 6.0, 1.0, 5.0);
    heightMapBuilder.Build();

    utils::RendererImage renderer;
    utils::Image image;
    renderer.SetSourceNoiseMap(heightMap);
    renderer.SetDestImage(image);
    /*
    renderer.ClearGradient();
    renderer.AddGradientPoint(-1.0000, utils::Color(  0,   0, 128, 255));
    renderer.AddGradientPoint(-0.2500, utils::Color(  0,   0, 255, 255));
    renderer.AddGradientPoint( 0.0000, utils::Color(  0, 128, 255, 255));
    renderer.AddGradientPoint( 0.0625, utils::Color(240, 240,  64, 255));
    renderer.AddGradientPoint( 0.1250, utils::Color( 32, 160,   0, 255));
    renderer.AddGradientPoint( 0.3750, utils::Color(224, 224,   0, 255));
    renderer.AddGradientPoint( 0.7500, utils::Color(128, 255, 128, 255));
    renderer.AddGradientPoint( 1.0000, utils::Color(255, 255, 255, 255));
    */
    renderer.Render();

    utils::WriterBMP writer;
    writer.SetSourceImage(image);
    writer.SetDestFilename("tutorial.bmp");
    writer.WriteDestFile();

    return 0;
}
