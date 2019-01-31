#include <iostream>
#include <cmath>
#include <vector>
#include <utility>
#include <queue>
#include <cstring>
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

using namespace std;
using namespace cv;

#define BIN_WIDTH 1                // quanti gradi all'interno della stessa categoria di voto
#define NUM_BINS 180 / BIN_WIDTH   // numero di categorie

/* === PARAMETRI PER CANNY EDGE DETECTION === */

#define KERNEL_SIZE 3
#define TRESHOLD 50
#define RATIO 3

void detectEdge(const Mat& in, Mat& out);

ostream& operator<<(ostream& out, vector<vector<int>> v);

void polarToCartesian(double rho, int theta, Point& p1, Point& p2);

int main(int argc, char** argv) {

    int i, j;
    int theta;      // parametro di angolo di inclinazione nel sistema di coordinate polari
    double rho;     // parametro di distanza (rho) nel sistema di coordinate polari

    Mat source, edges, output;

    if(argc < 3) {
        printf("USAGE: hough [fileName] [treshold]\n");
        return EXIT_FAILURE;
    }

    int lineTreshold = atoi(argv[2]);

    deque<pair<int, int>> edgePoints;  // <row, col>

    source = imread(argv[1], IMREAD_GRAYSCALE);
    namedWindow("source image", WINDOW_NORMAL | WINDOW_KEEPRATIO);

    output = source.clone();
    namedWindow("output image", WINDOW_NORMAL | WINDOW_KEEPRATIO);

    int maxDistance = hypot(source.rows, source.cols);

    cout<< maxDistance <<"\n";

    // matrice di voti
    vector<vector<int>> votes(2 * maxDistance, vector<int>(NUM_BINS, 0));

    cout<< votes.size() << ", " << votes[0].size() << endl;

    // downsample: tengo solo i bordi dell'immagine
    detectEdge(source, edges);
    namedWindow("edge detection result", WINDOW_NORMAL | WINDOW_KEEPRATIO);

    // vote

    for(i = 0; i < edges.rows; ++i) {
        for(j = 0; j < edges.cols; ++j) {

            if(edges.at<uchar>(i, j) == 255) {  // se incontro edge point

                // guarda gli angoli [-90°, +90°]
                for(theta = 0; theta <= 180; theta += BIN_WIDTH) {

                    rho = round(j * cos(theta - 90) + i * sin(theta - 90)) + maxDistance;

                    //cout<< rho <<endl;

                    votes[rho][theta]++;
                }
            }
        }
    }

    //cout<< votes <<"\n";

    // find peaks
    for(i = 0; i < votes.size(); ++i) {
        for(j = 0; j < votes[i].size(); ++j) {

            if(votes[i][j] >= lineTreshold) {

                rho = i - maxDistance;
                theta = j - 90;

                cout << "found line with rho = " << rho << " and theta = " << theta << "\n";

                // converti linea da polare a cartesiana

                Point p1, p2;   // 2 punti che descrivono la linea
		polarToCartesian(rho, theta, p1, p2);

                //cout<< p1 << ", " << p2 <<"\n";

                line(output, p1, p2, Scalar(0, 0, 255), 2, LINE_AA);

            }
        }
    }

    Point dummy1(10, 10), dummy2(100, 100);

    line(output, dummy1, dummy2, Scalar(0, 0, 255), 1, LINE_AA);

    imshow("source image", source);
    imshow("output image", output);
    imshow("edge detection result", edges);

    waitKey();


    return 0;
}

void detectEdge(const Mat& in, Mat& out) {

	blur(in, out, Size(3, 3));  // per immunità al rumore, sfocatura

	Canny(out, out, TRESHOLD, TRESHOLD*RATIO, KERNEL_SIZE);
}

ostream& operator<<(ostream& out, vector<vector<int>> v) {

	for (int i = 0; i < v.size(); ++i) {
		for (int j = 0; j < v[i].size(); ++j) {
			cout << v[i][j] << " ";
		}
		cout << "\n";
	}

	return out;
}

void polarToCartesian(double rho, int theta, Point& p1, Point& p2) {

	int x0 = cvRound(rho * cos(theta));
	int y0 = cvRound(rho * sin(theta));

	p1.x = cvRound(x0 + 1000 * (-sin(theta)));
	p1.y = cvRound(y0 + 1000 * (cos(theta)));

	p2.x = cvRound(x0 - 1000 * (-sin(theta)));
	p2.y = cvRound(y0 - 1000 * (cos(theta)));
}
