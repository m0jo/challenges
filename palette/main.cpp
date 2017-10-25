#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <cfloat>
#include <vector>
#include <cstring>
#include <ivt/Image/ByteImage.h>
#include <ivt/Image/ImageProcessor.h>
#include <ivt/Image/PrimitivesDrawer.h>

struct color{
    unsigned char r,g,b;
};
typedef std::pair<std::vector<color*>, color*> cluster;

color* averageColor(const std::vector<color*>* cl){
    color* ret = new color;
    double size = cl->size();
    double r,g,b;

    r = 0;
    g = 0;
    b = 0;

    for(color* c : *cl){
        r += c->r / size;
        g += c->g / size;
        b += c->b / size;
    }

    ret->r = r;
    ret->g = g;
    ret->b = b;

    return ret;
}

float distance(const color* a, const color* b){
    return sqrt(pow((double)(a->r - b->r),2.0) + pow((double)(a->g - b->g),2.0) + pow((double)(a->b - b->b), 2.0));
}

void getAllColors(const CByteImage* img, std::vector<color*>& colors){
    for(int index = 0; index < (img->width * img->height * img->bytesPerPixel); index+=3){
        color* c = new color;

        c->r = img->pixels[index];
        c->g = img->pixels[index+1];
        c->b = img->pixels[index+2];

        colors.push_back(c);
    }
}


int main(int argc, char *argv[])
{
    if(argc != 4){
        std::cerr << "To few arguments!" << std::endl;
        std::cerr << "Usage: " << argv[0] << "<intput image (bitmap)> <number of clusters> <outputimage (bitmap)>" << std::endl;
        return 1;
    }
    srand(time(NULL));

    CByteImage inputImage;
    inputImage.LoadFromFile(argv[1]);

    std::vector<color*> allcolors;
    getAllColors(&inputImage, allcolors);

    const int N_CLUSTERS = atoi(argv[2]);
    cluster clusters[N_CLUSTERS];

    //Initialize clusters with random color seed from inputimage
    for(cluster& cl : clusters)
        cl.second = allcolors[(rand()%allcolors.size())];

    size_t sizes[N_CLUSTERS] = {1};

    while(true){

        //Save cluster sizes then clear clusters
        for(int i=0; i<N_CLUSTERS; ++i){
            sizes[i] = clusters[i].first.size();
            clusters[i].first.clear();
        }

        //Compute distance of each color to each cluster center,
        //add color to cluster with smallest distance
        for(color* c : allcolors){

            float dist[N_CLUSTERS];
            for(int i=0; i<N_CLUSTERS; ++i)
                dist[i] = distance(c, clusters[i].second);

            float smallest = FLT_MAX;
            for(float& crnt_dist : dist)
                smallest = crnt_dist < smallest ? crnt_dist : smallest;

            for(int i=0;i<N_CLUSTERS;++i){
                if(((int)smallest) == ((int)dist[i])){
                    clusters[i].first.push_back(c);
                    break;
                }
            }
        }
        //Update average cluster color
        for(cluster& cl : clusters)
            cl.second = averageColor(&(cl.first));

        int n_same_size = 0;
        for(int i = 0; i<N_CLUSTERS; ++i)
            if(clusters[i].first.size() == sizes[i])
                n_same_size++;

        //Break if nothing changed during this iteration
        if(n_same_size == N_CLUSTERS)
            break;
    }

    //Initialize blank outputimage, copy inputimage to top
    CByteImage outputImage(inputImage.width, inputImage.height+100, inputImage.type);
    memset(outputImage.pixels, 255, outputImage.width*outputImage.height*outputImage.bytesPerPixel);
    memcpy(outputImage.pixels, inputImage.pixels, inputImage.width*inputImage.height*inputImage.bytesPerPixel);

    //Draw clusters
    Rectangle2d r;
    Vec2d c;
    for(int i=0;i<N_CLUSTERS;++i){
        r.width = 25;
        r.height = 40;
        c.y = inputImage.height + 50;
        c.x = ((outputImage.width/2)-N_CLUSTERS*r.width)+(i*r.width)+75;
        r.center = c;
        r.angle = 0;
        PrimitivesDrawer::DrawRectangle(&outputImage, r, clusters[i].second->r, clusters[i].second->g, clusters[i].second->b, -1);
    }

    outputImage.SaveToFile(argv[3]);


    return 0;
}
