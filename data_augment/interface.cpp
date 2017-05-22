#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include<ctime>


struct ImageStruct
{  
    int width;  
    int height;
    int channels; 
    char* pixels_buf;
};

extern "C" 
{
    // void test(ImageStruct p);
  void sim_light(ImageStruct p, int direction = 1);
  void adjust_color(ImageStruct p);
  void add_gaussian_noise(ImageStruct p);
  void add_pepper_noise(ImageStruct p);
  void random_blur(ImageStruct p);
  void random_rotate(ImageStruct p);
  void flip(ImageStruct p, int method);
}

void encode_image(cv::Mat image, ImageStruct p) {
  // ImageStruct p;
  p.width = image.cols;
  p.height = image.rows;
  p.channels = 3;
  for(int i = 0; i < p.height; ++i)
    for(int j = 0; j < p.width; ++j)
      for(int k = 0; k < 3; ++k)
      {
        int index = k + j * 3 + i * p.width * 3;
        p.pixels_buf[index] = image.at<cv::Vec3b>(i, j)[k];
        // std::cout << int(p.pixels_buf[index]) << std::endl;
      }
  // return p;
}

cv::Mat decode_image(ImageStruct p) {
  cv::Mat new_image(p.height, p.width, CV_8UC3,cv::Scalar(0,0,255));
  // std::cout << p.width * p.height * p.channels << std::endl;
  for(int i = 0; i < p.width * p.height * p.channels; ++i) {
    int index_i = i / (p.width * 3);
    int index_j = (i % (p.width * 3)) / 3;
    int index_k = i % 3;
    new_image.at<cv::Vec3b>(index_i, index_j)[index_k] = p.pixels_buf[i];
  }
  // cv::imshow("main2", new_image);
  // cv::waitKey();
  return new_image;
}


cv::Mat SimulateIllumination(cv::Mat image, int direction, bool is_large_scale) {
  double bright_factor = 0.0;
  if(!is_large_scale)
  {
    bright_factor = (rand() % 10 + 8.0) / 10.0;
  }
  else
  {
    bright_factor = (rand() % 10 + 5.0) / 10.0;
  }
  // cout << "bright_factor: " << bright_factor << endl;
  cv::Mat new_image = cv::Mat::zeros(image.size(), image.type());
  int width = image.cols;
  int height = image.rows;
  // cout << "image size: " << image.size() << endl;
  // cv::Mat new_image(width, height, CV_8UC3, cv::Scalar(0,0,0));

  double alpha = 0.0;
  double offset = (rand() % 30) / 30.0;

  switch(direction){
    case 0:
      for(int i = 0; i < width; ++i)
      {
        alpha = MAX(((width - 2.0 * i) / (width * 1.0)) * bright_factor, -0.25) + offset;
        // cout << "alpha: " << alpha << endl;
        for(int j = 0; j < height; ++j)
        {
          for(int k = 0; k < 3; ++k)
          {
            int pixel = MIN((1.0 + alpha) * image.at<cv::Vec3b>(j, i)[k], 255);
            new_image.at<cv::Vec3b>(j, i)[k] = cv::saturate_cast<uchar>(pixel);
          }
        }
      }
      break;
    case 1:
      for(int i = 0; i < width; ++i)
      {
        alpha = MIN(((width - 2.0 * i) / (width * 1.0)) * bright_factor, 0.25) - offset;
        // cout << "alpha: " << alpha << endl;
        for(int j = 0; j < height; ++j)
        {
          for(int k = 0; k < 3; ++k)
          {
            int pixel = MIN((1.0 - alpha) * image.at<cv::Vec3b>(j, i)[k], 255);
            new_image.at<cv::Vec3b>(j, i)[k] = cv::saturate_cast<uchar>(pixel);
          }
        }
      } 
      break;

    case 2:
      for(int i = 0; i < height; ++i)
      {
        alpha = MAX(((height - 2.0 * i) / (height * 1.0)) * bright_factor, -0.25) + offset;
        // cout << "alpha: " << alpha << endl;
        for(int j = 0; j < width; ++j)
        {
          for(int k = 0; k < 3; ++k)
          {
            int pixel = MIN((1.0 + alpha) * image.at<cv::Vec3b>(i, j)[k], 255);
            new_image.at<cv::Vec3b>(i, j)[k] = cv::saturate_cast<uchar>(pixel);
          }
        }
      }
      break;

    case 3:
      for(int i = 0; i < height; ++i)
      {
        alpha = MIN(((height - 2.0 * i) / (height * 1.0)) * bright_factor, 0.25) - offset;
        // cout << "alpha: " << alpha << endl;
        for(int j = 0; j < width; ++j)
        {
          for(int k = 0; k < 3; ++k)
          {
            int pixel = MIN((1.0 - alpha) * image.at<cv::Vec3b>(i, j)[k], 255);
            new_image.at<cv::Vec3b>(i, j)[k] = cv::saturate_cast<uchar>(pixel);
          }
        }
      }
      break;
  }
  return new_image;
}

cv::Mat ColorJittering(cv::Mat image, bool is_large_scale) {
  cv::Mat new_image = cv::Mat::zeros(image.size(), image.type());
  double contrast_fac = 0.0;
  int bright_fac = 0;
  if(!is_large_scale)
  {
    contrast_fac = (rand() / double(RAND_MAX)) * 1.0 + 0.75;
    bright_fac = rand() % 150 - 85;
  }
  else
  {
    contrast_fac = (rand() / double(RAND_MAX)) * 2.0 + 1.0;
    bright_fac = rand() % 100;
  }
  // cout << "contrast_fac: " << contrast_fac << "bright_fac: " << bright_fac << endl;
  for(int y = 0; y < image.rows; y++)
  {
    for(int x = 0; x < image.cols; x++)
    {
      for(int c = 0; c < 3; c++)
      {
        int pixel = contrast_fac * (image.at<cv::Vec3b>(y,x)[c]) + bright_fac;
        new_image.at<cv::Vec3b>(y,x)[c] = cv::saturate_cast<uchar>(pixel);
      }
    }
  }
  return new_image;
}

double generateGaussianNoise(double mu, double sigma)  
{  
    const double epsilon = std::numeric_limits<double>::min(); 
    static double z0, z1;  
    static bool flag = false;  
    flag = !flag;  
    if (!flag)  
        return z1*sigma + mu;  
    double u1, u2;  
      
    do  
    {  
        u1 = rand()*(1.0 / RAND_MAX);  
        u2 = rand()*(1.0 / RAND_MAX);  
    } while (u1 <= epsilon);  
    z0 = sqrt(-2.0*log(u1))*cos(2 * CV_PI * u2);  
    z1 = sqrt(-2.0*log(u1))*sin(2 * CV_PI * u2);  
    return z1*sigma + mu;  
}
  
cv::Mat AddGaussianNoisy(cv::Mat srcImage, bool is_large_scale) {
    cv::Mat resultImage = srcImage.clone();
    int channels = resultImage.channels();
    int nRows = resultImage.rows;
    int nCols = resultImage.cols*channels;
    if (resultImage.isContinuous())
    {  
        nCols *= nRows;
        nRows = 1;
    }
    for (int i = 0; i < nRows; i++)
    {
        for (int j = 0; j < nCols; j++)
        {
            int val = resultImage.ptr<uchar>(i)[j] + generateGaussianNoise(2, 0.8) * 32 * 0.5;
            if (val < 0) 
                val = 0;
            if (val > 255) 
                val = 255;
            resultImage.ptr<uchar>(i)[j] = (uchar)val;
        }
    }
    return resultImage;
}

cv::Mat AddPepperSaltNoisy(cv::Mat image, bool is_large_scale) {
  cv::Mat new_image = cv::Mat::zeros(image.size(), image.type());
  int pixel_cnt = image.rows * image.cols;
  int skip_count = 0;
  int rand_skip = rand() % 80;
  int white_or_black = rand() % 2;
  bool is_checkpoint = false;

  for(int y = 0; y < image.rows; y++)
  {
    for(int x = 0; x < image.cols; x++)
    {
      skip_count ++;
      if(skip_count > rand_skip) {
        is_checkpoint = true;
        rand_skip = rand() % 80;
        white_or_black = rand() % 2;
        skip_count = 0;
      }
      for(int c = 0; c < 3; c++)
      {
        int pixel = 0;
        if(is_checkpoint) {
            pixel = white_or_black == 0? 255:0;
        }
        else {
            pixel = image.at<cv::Vec3b>(y,x)[c];
        }
        new_image.at<cv::Vec3b>(y,x)[c] = cv::saturate_cast<uchar>(pixel);
      }
      is_checkpoint = false;
    }
  }
 
  return new_image;
}

cv::Mat ScaleUpDown(cv::Mat image, float shrink_ratio_min, 
                                    float shrink_ratio_max, int shrink_method, int enlarge_method) {
  cv::Mat cv_img = image.clone();
  cv::Mat shrink_img;

  int rand_times = rand() % 5;

  for(int i = 0; i < rand_times; ++i) {
    
      float shrink_ratio = (rand() / (RAND_MAX + 1.0)) * (shrink_ratio_max - shrink_ratio_min) + shrink_ratio_min;
    
      int  origin_width = cv_img.cols, origin_height = cv_img.rows;
      int shrink_width = round(shrink_ratio * origin_width);
      int shrink_height = round(shrink_ratio * origin_height);
      int shrink_interpolation = CV_INTER_LINEAR, enlarge_interpolation = CV_INTER_LINEAR;
    
      switch(shrink_method) {
        case 0:
          shrink_interpolation = CV_INTER_NN;
          break;
        case 1:
          shrink_interpolation = CV_INTER_LINEAR;
          break;
        case 2:
          shrink_interpolation = CV_INTER_CUBIC;
          break;
        case 3:
          shrink_interpolation = CV_INTER_AREA;
          break;
        case 4:
          shrink_interpolation = CV_INTER_LANCZOS4;
          break;
        case 5:
          switch(rand() % 5){
            case 0 : {shrink_interpolation = CV_INTER_NN; break;}
            case 1 : {shrink_interpolation = CV_INTER_LINEAR ; break;}
            case 2 : {shrink_interpolation = CV_INTER_CUBIC  ; break;}
            case 3 : {shrink_interpolation = CV_INTER_AREA   ; break;}
            case 4 : {shrink_interpolation = CV_INTER_LANCZOS4; break;}
            default: break;
          }
      }
    
      cv::resize(cv_img, shrink_img, cv::Size(shrink_width, shrink_height), 0.0, 0.0, shrink_interpolation);
    
      switch(enlarge_method) {
        case 0:
          enlarge_interpolation = CV_INTER_NN;
          break;
        case 1:
          enlarge_interpolation = CV_INTER_LINEAR;
          break;
        case 2:
          enlarge_interpolation = CV_INTER_CUBIC;
          break;
        case 3:
          enlarge_interpolation = CV_INTER_AREA;
          break;
        case 4:
          enlarge_interpolation = CV_INTER_LANCZOS4;
          break;
        case 5:
          switch(rand() % 5){
            case 0 : {enlarge_interpolation = CV_INTER_NN; break;}
            case 1 : {enlarge_interpolation = CV_INTER_LINEAR ; break;}
            case 2 : {enlarge_interpolation = CV_INTER_CUBIC  ; break;}
            case 3 : {enlarge_interpolation = CV_INTER_AREA   ; break;}
            case 4 : {enlarge_interpolation = CV_INTER_LANCZOS4; break;}
            default: break;
          }
      }
    
      cv::resize(shrink_img, cv_img, cv::Size(origin_width, origin_height), 0.0, 0.0, enlarge_interpolation);
  }
  return cv_img;
}

cv::Mat RotateImage(cv::Mat image, bool is_large_scale) {
  cv::Mat new_image = cv::Mat::zeros(image.size(), image.type());
  double rand_angle = 0.0;
  double shrink_scale = 1.0;

  if(!is_large_scale)
  {
    rand_angle = (rand() / double(RAND_MAX)) * 30.0 - 15.0;
  }
  else
  {
    rand_angle = (rand() / double(RAND_MAX)) * 60.0 - 30.0;
  }
  cv::Point2f center = cv::Point2f(image.cols / 2, image.rows / 2);

  cv::Mat rotate_mat = cv::getRotationMatrix2D(center, rand_angle, shrink_scale);
  cv::warpAffine(image, new_image, rotate_mat, image.size());

  return new_image;
}

cv::Mat FlipImage(cv::Mat image, int flip_method) {
  cv::Mat new_image;
  cv::flip(image, new_image, flip_method);
  return new_image;
}

void sim_light(ImageStruct p, int direction) {
  // std::cout << "sim begin!" << std::endl;
  cv::Mat image = decode_image(p);
  // std::cout << "decode success!" << std::endl;
  cv::Mat new_image = SimulateIllumination(image, direction, false);
  encode_image(new_image, p);
  // cv::Mat image_test = decode_image(p);
  // cv::imshow("main", image_test);
  // cv::waitKey();
}

void adjust_color(ImageStruct p) {
  cv::Mat image = decode_image(p);
  // std::cout << "decode success!" << std::endl;
  cv::Mat new_image = ColorJittering(image, false);
  encode_image(new_image, p);
  // cv::Mat image_test = decode_image(p);
}

void add_gaussian_noise(ImageStruct p) {
  cv::Mat image = decode_image(p);
  // std::cout << "decode success!" << std::endl;
  cv::Mat new_image = AddGaussianNoisy(image, false);
  encode_image(new_image, p);
 
}

void add_pepper_noise(ImageStruct p) {
  cv::Mat image = decode_image(p);
  // std::cout << "decode success!" << std::endl;
  cv::Mat new_image = AddPepperSaltNoisy(image, false);
  encode_image(new_image, p);
 
}

void random_blur(ImageStruct p) {
  cv::Mat image = decode_image(p);
  // std::cout << "decode success!" << std::endl;
  cv::Mat new_image = ScaleUpDown(image, 0.30, 0.86, 5, 1);
  encode_image(new_image, p);
}

void random_rotate(ImageStruct p) {
  cv::Mat image = decode_image(p);
  // std::cout << "decode success!" << std::endl;
  cv::Mat new_image = RotateImage(image, false);
  encode_image(new_image, p); 
}

void flip(ImageStruct p, int method) {
  cv::Mat image = decode_image(p);
  // std::cout << "decode success!" << std::endl;
  cv::Mat new_image = FlipImage(image, method);
  encode_image(new_image, p);
}
