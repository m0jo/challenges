#include <iostream>
#include <math.h>
#include <ivt/Image/ByteImage.h>
#include <ivt/Image/ImageProcessor.h>
#include <ivt/Image/PrimitivesDrawer.h>
#include <stdlib.h>

struct color{
    unsigned char r,g,b;
};

//Compute pixel distance
inline double diffPixel(const CByteImage* src, const CByteImage* img, int x, int y){
    if(x > img->width || x < 0 || y > img->height || y < 0)
        return 99999999999999999.9;
    int index = 3 * (y * src->width + x);
    return sqrt(pow((double) (src->pixels[index]   - img->pixels[index]), 2.0) +
                pow((double) (src->pixels[index+1] - img->pixels[index+1]), 2.0) +
                pow((double) (src->pixels[index+2] - img->pixels[index+2]), 2.0));
}

//Compute line difference
double diffLine(const CByteImage* src, const CByteImage* img, Vec2d* p1, Vec2d* p2){
    if(p1->x > img->width || p2->x > img->width || p1->y < 0 || p2->y < 0)
        return 99999999999999999.9;

    const float dx = p1->x - p2->x;
    const float dy = p1->y - p2->y;
    double sum = 0;

    if (fabsf(dy) < fabsf(dx)){
        const float slope = dy / dx;
        const int max_x = int(p2->x + 0.5f);
        float y = p1->y + 0.5f;

        if (p1->x < p2->x)
            for (int x = int(p1->x + 0.5f); x <= max_x; x++, y += slope)
                sum += diffPixel(src, img, x, y);
        else
            for (int x = int(p1->x + 0.5f); x >= max_x; x--, y -= slope)
                sum += diffPixel(src, img, x, y);

    }else{
        const float slope = dx / dy;
        const int max_y = int(p2->y + 0.5f);
        float x = p1->x + 0.5f;

        if (p1->y < p2->y)
            for (int y = int(p1->y + 0.5f); y <= max_y; y++, x += slope)
                sum += diffPixel(src, img, x, y);
        else
            for (int y = int(p1->y + 0.5f); y >= max_y; y--, x -= slope)
                sum += diffPixel(src, img, x, y);
    }

    return sum;

}

int main(int argc, char *argv[])
{
    if(argc != 4){
        std::cerr<< "ERR: Invalid arguments!\n\rUsage: "<< argv[0] << " <input.bmp> <iterations> <output.bmp>\n\r" << std::endl;
        return 1;
    }
    srand(time(NULL));

    //Load file
    CByteImage src;
    src.LoadFromFile(argv[1]);
    int width = src.width;
    int height = src.height;

    //Create blank images (black)
    CByteImage img1(&src);
    CByteImage img2(&src);
    ImageProcessor::Zero(&img1);
    ImageProcessor::Zero(&img2);

    //Variables used inside loop
    color c;
    Vec2d line_coord_1;
    Vec2d line_coord_2;
    unsigned int index;
    MyRegion ROI;

    int iterations = atoi(argv[2]);

    std::cout << "-*- Starting to sketch "<< argv[1] << "\n\r-*- Using " << iterations << " iterations.." << std::endl;
    for(int i=0;i<iterations;++i){

        //Generate line with random coordinates and color from inputimage
        line_coord_1.x = (float) (rand()%width);
        line_coord_1.y = (float) (rand()%height);
        line_coord_2.x = line_coord_1.x + (rand()%50);
        line_coord_2.y = line_coord_1.y + (rand()%50);
        index = 3*(rand() % ((src.width*src.height))/3);
        c.r = src.pixels[index];
        c.g = src.pixels[index+1];
        c.b = src.pixels[index+2];

        PrimitivesDrawer::DrawLine(&img1, line_coord_1, line_coord_2, c.r, c.g, c.b);

        if(line_coord_1.x < line_coord_2.x){
            ROI.min_x = line_coord_1.x;
            ROI.max_x = line_coord_2.x+1;
        }else{
            ROI.min_x = line_coord_2.x;
            ROI.max_x = line_coord_1.x+1;
        }
        if(line_coord_1.y < line_coord_2.y){
            ROI.min_y = line_coord_1.y;
            ROI.max_y = line_coord_2.y+1;
        }else{
            ROI.min_y = line_coord_2.y;
            ROI.max_y = line_coord_1.y+1;
        }

        //Check whether we are converging towards original image
        if(diffLine(&src, &img1, &line_coord_1, &line_coord_2) < diffLine(&src, &img2, &line_coord_1, &line_coord_2))
            //yes we are
            ImageProcessor::CopyImage(&img1, &img2, &ROI, true);
        else
            //nope, sorry
            ImageProcessor::CopyImage(&img2, &img1, &ROI, true);


    }
    std::cout << "-*- Done" << std::endl;
    img1.SaveToFile(argv[3]);

    return 0;
}
