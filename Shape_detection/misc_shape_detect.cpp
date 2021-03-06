#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cmath>
#include <iostream>

static double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

void setLabel(cv::Mat& im, const std::string label, std::vector<cv::Point>& contour)
{
	int fontface = cv::FONT_HERSHEY_SIMPLEX;
	double scale = 0.4;
	int thickness = 1;
	int baseline = 0;

	cv::Size text = cv::getTextSize(label, fontface, scale, thickness, &baseline);
	cv::Rect r = cv::boundingRect(contour);

	cv::Point pt(r.x + ((r.width - text.width) / 2), r.y + ((r.height + text.height) / 2));
	cv::rectangle(im, pt + cv::Point(0, baseline), pt + cv::Point(text.width, -text.height), CV_RGB(255,255,255), CV_FILLED);
	cv::putText(im, label, pt, fontface, scale, CV_RGB(0,0,0), thickness, 8);
}

cv::Mat main_detector(cv::Mat& src , int lcanny , int ucanny)
{
        cv::Mat gray;
	cv::cvtColor(src, gray, CV_BGR2GRAY);

	cv::Mat bw;
	cv::Canny(gray, bw, lcanny , ucanny, 5);  //kernel_size=5
        cv::imshow("canny_output" , bw );

	// Find contours
	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(bw.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	std::vector<cv::Point> approx;
	cv::Mat dst = src.clone();

	for (int i = 0; i < contours.size(); i++)
	{
	        cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);

		// Skip small or non-convex objects 
		if (std::fabs(cv::contourArea(contours[i])) < 30 || !cv::isContourConvex(approx))
			continue;

		
		else if (approx.size() >= 3 && approx.size() <= 6)
		{
			// Number of vertices of polygonal curve
			int vtc = approx.size();

			// Get the cosines of all corners
			std::vector<double> cos;
			for (int j = 2; j < vtc+1; j++)
				cos.push_back(angle(approx[j%vtc], approx[j-2], approx[j-1]));

			// Sort ascending the cosine values
			std::sort(cos.begin(), cos.end());

			// Get the lowest and the highest cosine
			double mincos = cos.front();
			double maxcos = cos.back();

			// Use the degrees obtained above and the number of vertices
			// to determine the shape of the contour
			if (vtc == 4 && mincos >= -0.1 && maxcos <= 0.3)
				{
                                 //setLabel(dst, "RECT", contours[i]);
                                 cv::polylines( dst , approx , true , cv::Scalar(0,255,0) , 3 , CV_AA , 0 );
                                }
                        else if (vtc == 3 && mincos >= 0.25 && maxcos <= 0.750)
                                {
                                 //setLabel(dst, "TRI", contours[i]);
                                 cv::polylines( dst , approx , true , cv::Scalar(0,255,0) , 3 , CV_AA , 0 );
                                }
			else if (vtc == 5 && mincos >= -0.34 && maxcos <= -0.27)
                                {
				//setLabel(dst, "PENTA", contours[i]);
                                cv::polylines( dst , approx , true , cv::Scalar(0,255,0) , 3 , CV_AA , 0 );
                                }
			else if (vtc == 6 && mincos >= -0.55 && maxcos <= -0.45)
				{
                                 //setLabel(dst, "HEXA", contours[i]);
                                 cv::polylines( dst , approx , true , cv::Scalar(0,255,0) , 3 , CV_AA , 0 );
                                }
                                 
		}
		else
		{
			
			double area = cv::contourArea(contours[i]);
			cv::Rect r = cv::boundingRect(contours[i]);
			int radius = r.width / 2;
                        

			if (std::abs(1 - ((double)r.width / r.height)) <= 0.2 &&
			    std::abs(1 - (area / (CV_PI * std::pow(radius, 2)))) <= 0.2)
				//setLabel(dst, "CIR", contours[i]);
                                cv::polylines( dst , approx , true , cv::Scalar(0,255,0) , 3 , CV_AA , 0 );
		}
	}

        return dst;
	


}
int main(int argc , char** argv)
{

	cv::Mat original = cv::imread(argv[1] , 1 );
	if (original.empty())
		return -1;
        int lcanny=50;
        int ucanny=50;
        int maxlcanny = 600 , maxucanny = 600;
        cv::namedWindow("Shapes_detected" , CV_WINDOW_AUTOSIZE);
        cv::namedWindow("canny_output" , CV_WINDOW_AUTOSIZE);
        cv::createTrackbar( "canny_lower_value" , "Shapes_detected" , &lcanny , maxlcanny);
        cv::createTrackbar( "canny_upper_value" , "Shapes_detected" , &ucanny , maxucanny);

        while(1)
        {
        cv::Mat final=main_detector(original , lcanny , ucanny);
	cv::imshow("Shapes_detected", final);

	char a = cv::waitKey(33);
        }
	return 0;
}
