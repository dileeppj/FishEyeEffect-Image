/****************************************************************************************************
										Fish Eye Effect
									   [ CPP and OpenCV ]
*****************************************************************************************************/

/**	Header Files	*/
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string.h>
#include <conio.h>

using namespace std;
using namespace cv;

/**	Function to get the file extension*/
string getFileExt(const string& s) 
{
	size_t i = s.rfind('.', s.length());
	if (i != string::npos) 
	{
		return(s.substr(i + 1, s.length() - i));
	}

	return("");
}

/**	Function to calculate shift*/
float calc_shift(float x1, float x2, float cx, float k)
{
	float thresh = 1;
	float x3 = x1 + (x2 - x1)*0.5;
	float res1 = x1 + ((x1 - cx)*k*((x1 - cx)*(x1 - cx)));
	float res3 = x3 + ((x3 - cx)*k*((x3 - cx)*(x3 - cx)));

	if (res1>-thresh && res1 < thresh)
		return x1;
	if (res3<0) 
	{
		return calc_shift(x3, x2, cx, k);
	}
	else 
	{
		return calc_shift(x1, x3, cx, k);
	}
}

float getRadialX(float x, float y, float cx, float cy, float k, bool scale, Vec4f props)
{
	float result;
	if (scale)
	{
		float xshift = props[0];
		float yshift = props[1];
		float xscale = props[2];
		float yscale = props[3];

		x = (x*xscale + xshift);
		y = (y*yscale + yshift);
		result = x + ((x - cx)*k*((x - cx)*(x - cx) + (y - cy)*(y - cy)));
	}
	else
	{
			result = x + ((x - cx)*k*((x - cx)*(x - cx) + (y - cy)*(y - cy)));
	}
	return result;
}

float getRadialY(float x, float y, float cx, float cy, float k, bool scale, Vec4f props)
{
	float result;
	if (scale)
	{
		float xshift = props[0];
		float yshift = props[1];
		float xscale = props[2];
		float yscale = props[3];

		x = (x*xscale + xshift);
		y = (y*yscale + yshift);
		result = y + ((y - cy)*k*((x - cx)*(x - cx) + (y - cy)*(y - cy)));
	}
	else 
	{
		result = y + ((y - cy)*k*((x - cx)*(x - cx) + (y - cy)*(y - cy)));
	}
	return result;
}

//	Fish Eye Function							Cx, Cy Center of x & y
//	_src : Input image, _dst : Output image, Cx,Cy coordinates from where the distorted image will have as initial point, k : distortion factor
void fishEye(InputArray _src, OutputArray _dst, double Cx, double Cy, double k, bool scale = true)
{
	Mat src = _src.getMat();
	Mat mapx = Mat(src.size(), CV_32FC1);
	Mat mapy = Mat(src.size(), CV_32FC1);

	int w = src.cols;	//	Width
	int h = src.rows;	//	Height

	Vec4f props;
	//Calculating x and y shifts to be applied
	float xShift = calc_shift(0, Cx - 1, Cx, k);
	props[0] = xShift;
	float newCenterX = w - Cx;
	float xShift2 = calc_shift(0, newCenterX - 1, newCenterX, k);

	float yShift = calc_shift(0, Cy - 1, Cy, k);
	props[1] = yShift;
	float newCenterY = w - Cy;
	float yShift2 = calc_shift(0, newCenterY - 1, newCenterY, k);
	
	//	Calculating the scale factor from the x & y shifts accordingly
	float xScale = (w - xShift - xShift2) / w;
	props[2] = xScale;
	float yScale = (h - yShift - yShift2) / h;
	props[3] = yScale;

	float* p = mapx.ptr<float>(0);

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			*p++ = getRadialX((float)x, (float)y, Cx, Cy, k, scale, props);
		}
	}

	p = mapy.ptr<float>(0);

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			*p++ = getRadialY((float)x, (float)y, Cx, Cy, k, scale, props);
		}
	}

	remap(src, _dst, mapx, mapy, CV_INTER_LINEAR, BORDER_CONSTANT);

}

/**	Main Function*/
int main()
{
	string ext,file_name;
	cout << "\n\t\t\tFish Eye Effect";
	cout << "\n\n Enter the file name : ";
	cin >> file_name;
	
	ext = getFileExt(file_name);	//	Check if the file name provided is of an image
	if (!ext.compare("jpg") || !ext.compare("jpeg") || !ext.compare("png") || !ext.compare("bmp"))
	{
		Mat input_image = imread(file_name, 1);	//	Read the image
		Mat output_image;
		Size sz = input_image.size();
		
		// Fish Eye Effect looks good in Square shape
		if (sz.height != sz.width)
		{
			cout << "\n Resizing the image to square ";	// Converting to square shape
			double mx = max(sz.height, sz.width);
			if (mx >= 1000)								// Resizing large images (>1000px) to fit the screen
			{
				mx = mx / 2;
			}
			Size nw_sz;
			nw_sz.height = nw_sz.width = mx;
			cout << nw_sz;
			resize(input_image, input_image, nw_sz, 0, 0, 3);
		}

		imshow("Input Image", input_image);

		fishEye(input_image, output_image, input_image.cols / 2, input_image.rows / 2, 0.0001);	// Fish Eye Function

		imshow("Output Image", output_image);

		waitKey(0);
	}

	else
	{
		cout << "\n\t\tProblem loading image!!!\n Enter Image file name with extensions (*.jpg,*.jpeg,*.png)";
		_getch();
	}

	return 0;
}