#include <cmath>
#include <iostream>
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/rect.pb.h"

namespace mediapipe
{

namespace
{
constexpr char normRectTag[] = "NORM_RECT";
constexpr char landmarkListTag[] = "LANDMARKS";
constexpr char recognizedHandGestureTag[] = "RECOGNIZED_HAND_GESTURE";
}

class HandGestureRecognitionCalculator : public CalculatorBase
{
public:
    static ::mediapipe::Status GetContract(CalculatorContract *cc);
    ::mediapipe::Status Open(CalculatorContext *cc) override;
    ::mediapipe::Status Process(CalculatorContext *cc) override;

private:
    int getPalmDirection(float rotation){
        int direction = 0;
	if (rotation > -0.4 && rotation < 0.8){
	    direction = 0; //up
	}else if (rotation > 1.6 || rotation < -1.6){
	    direction = 1;  //down
	}else{
	    direction = 2;
	}
	return direction;
    }

    float getTwoPointDistance(Landmark point1,Landmark point2){
        float distance = sqrt(pow(point1.x() - point2.x(), 2) + pow(point1.y() - point2.y(), 2) + pow(point1.z() - point2.z(), 2));
        return distance;
    }

    float getAngle(Landmark point1,Landmark point2,Landmark point3){
        float vec1[3] = {point1.x()-point2.x(),point1.y()-point2.y(),point1.z()-point2.z()};
        float vec2[3] = {point3.x()-point2.x(),point3.y()-point2.y(),point3.z()-point2.z()};
        float cosine = (vec1[0]*vec2[0]+vec1[1]*vec2[1]+vec1[2]*vec2[2])
               / sqrt(pow(vec1[0],2)+pow(vec1[1],2)+pow(vec1[2],2))
               / sqrt(pow(vec2[0],2)+pow(vec2[1],2)+pow(vec2[2],2));
        float acosine = acos(cosine);
        double pi = 3.141592653589793;
        float angle = 180 - (acosine / pi * 180);
        return angle;
    }

    int getThumbStatus(int palmDirection,Landmark point0,Landmark point1,Landmark point2,Landmark point3,Landmark point4,Landmark point5,Landmark point6,Landmark point10,Landmark point14,Landmark point18){
        float point2ToPoint4 = getTwoPointDistance(point2,point4);
	float point4ToPoint6 = getTwoPointDistance(point3,point6);
        float point4ToPoint10 = getTwoPointDistance(point4,point10);
	float point4ToPoint14 = getTwoPointDistance(point3,point14);
        float point4ToPoint18 = getTwoPointDistance(point3,point18);
        float sumAngle = getAngle(point2,point3,point4);
	float mipAngle = getAngle(point0,point2,point3);
	std::cout << "sumAngle :" << sumAngle << std::endl;
	std::cout << "mipAngle :" << mipAngle << std::endl;

        int fingerStatus = 1;
        if (palmDirection == 0){
	    if(sumAngle + mipAngle > 90 || point4ToPoint6 > point4ToPoint10 || point4ToPoint6  > point4ToPoint14 || point4ToPoint6 > point4ToPoint18  || point2ToPoint4 > point4ToPoint10 * 1.25 ){
                fingerStatus = 1; //close
            }else if(mipAngle < 20){
                fingerStatus = 0; //open
	    }else{
                fingerStatus = 2; //midclose
            }
	}else if(palmDirection == 2){
	    if(sumAngle + mipAngle > 95 || point4ToPoint6 > point4ToPoint10 || point4ToPoint6  > point4ToPoint14 || point4ToPoint6 > point4ToPoint18 || point2ToPoint4 > point4ToPoint10 *1.6){
                fingerStatus = 1; //close
            }else if(mipAngle < 15){
                fingerStatus = 0; //open
            }else{
                fingerStatus = 2; //midclose
            }
	}
        return fingerStatus;
    }

    int getFirstFingerStatus(int palmDirection,Landmark point2,Landmark point3,Landmark point5,Landmark point6,Landmark point7,Landmark point8){
        float point2ToPoint5 = getTwoPointDistance(point2,point5);
	float point2ToPoint8 = getTwoPointDistance(point2,point8);
   	float point2ToPoint3 = getTwoPointDistance(point2,point3);
	float point3ToPoint6 = getTwoPointDistance(point3,point6);
	float sumAngle = getAngle(point5,point6,point7) + getAngle(point6,point7,point8);
        float mipAngle = getAngle(point2,point5,point6);
        std::cout << "sumAngle :" << sumAngle << std::endl;
        std::cout << "mipAngle :" << mipAngle << std::endl;
        
        int fingerStatus;
	if (palmDirection == 0){
	    if(point2ToPoint5 > point2ToPoint8 || mipAngle > 80 || point2ToPoint3 > point3ToPoint6){
	        fingerStatus = 1;//close
	    }else if(point2ToPoint5 < point2ToPoint8 && sumAngle > 20){
	        fingerStatus = 2;//midclose
	    }else{
	        fingerStatus = 0;//open
	    }
	}else if (palmDirection == 2){
	    if(point2ToPoint5 > point2ToPoint8 || sumAngle > 120){
                fingerStatus = 1;//close
            }else if(mipAngle > 40){
                if(sumAngle < 35){
                    fingerStatus = 0;//open
                }else{
                    fingerStatus = 2;
                }
            }else if(mipAngle < 40){
                if(sumAngle < 25){
                    fingerStatus = 0;
                }else{
                    fingerStatus = 2;
                }
            }else{
                fingerStatus = 2;//midclose
            }
        }
        return fingerStatus;
    }

    int getSecondFingerStatus(int palmDirection,Landmark point0,Landmark point9,Landmark point10,Landmark point11,Landmark point12){
        float point0ToPoint9 = getTwoPointDistance(point0,point9);
    	float point0ToPoint10 = getTwoPointDistance(point0,point10);
        float point0ToPoint12 = getTwoPointDistance(point0,point12);
        float sumAngle = getAngle(point9,point10,point11) + getAngle(point10,point11,point12);
        float mipAngle = getAngle(point0,point9,point10);
        std::cout << "sumAngle :" << sumAngle << std::endl;
        std::cout << "mipAngle :" << mipAngle << std::endl;

	int fingerStatus;
        if (palmDirection == 0){
            if(point0ToPoint9 > point0ToPoint12 || mipAngle > 80){
                fingerStatus = 1;//close
            }else if(point0ToPoint9 < point0ToPoint12 && sumAngle > 20){
                fingerStatus = 2;//midclose
            }else if(point0ToPoint10 < point0ToPoint12 && sumAngle > 20){
	        fingerStatus = 2;//midclose
            }else{
                fingerStatus = 0;//open
	    }
        }else if (palmDirection == 2){
	    if(point0ToPoint9 > point0ToPoint12 || mipAngle > 80 || sumAngle > 140){
                fingerStatus = 1;//close
            }else if(mipAngle > 40){
                if(sumAngle < 35){
                    fingerStatus = 0;//open
                }else{
                    fingerStatus = 2;
                }
            }else if(mipAngle < 40){
                if(sumAngle < 25){
                    fingerStatus = 0;
                }else{
                    fingerStatus = 2;
                }
            }else{
                fingerStatus = 2;//midclose
            }
        }
        return fingerStatus;
    }

    int getThirdFingerStatus(int palmDirection,Landmark point0,Landmark point13,Landmark point14,Landmark point15,Landmark point16){
        float point0ToPoint13 = getTwoPointDistance(point0,point13);
        float point0ToPoint14 = getTwoPointDistance(point0,point14);
        float point0ToPoint16 = getTwoPointDistance(point0,point16);
        float sumAngle = getAngle(point13,point14,point15) + getAngle(point14,point15,point16);
        float mipAngle = getAngle(point0,point13,point14);
        std::cout << "sumAngle :" << sumAngle << std::endl;
        std::cout << "mipAngle :" << mipAngle << std::endl;

        int fingerStatus;
        if (palmDirection == 0){
            if(point0ToPoint13 > point0ToPoint16 || mipAngle > 80 || sumAngle > 140){
                fingerStatus = 1;//close
            }else if(point0ToPoint13 * 1.1 < point0ToPoint16 && sumAngle > 30){
                fingerStatus = 2;//midclose
            }else if(point0ToPoint14 * 1.1 < point0ToPoint16 && sumAngle > 30){
                fingerStatus = 2;//midclose
            }else{
                fingerStatus = 0;//open
	    }
        }else if (palmDirection == 2){
	    if(point0ToPoint13 > point0ToPoint16 || mipAngle > 80 || sumAngle > 140){
                fingerStatus = 1;//close
	    }else if(mipAngle > 40){
                if(sumAngle < 35){
                    fingerStatus = 0;//open
                }else{
		    fingerStatus = 2;
		}
            }else if(mipAngle < 40){
                if(sumAngle < 25){ 
                    fingerStatus = 0;
                }else{
	            fingerStatus = 2;
		}
            }else{
                fingerStatus = 2;//midclose
            }
        }
        return fingerStatus;
    }

    int getFourthFingerStatus(int palmDirection,Landmark point0,Landmark point17,Landmark point18,Landmark point19,Landmark point20){
        float point0ToPoint17 = getTwoPointDistance(point0,point17);
        float point0ToPoint18 = getTwoPointDistance(point0,point18);
        float point0ToPoint20 = getTwoPointDistance(point0,point20);
        float sumAngle = getAngle(point17,point18,point19) + getAngle(point18,point19,point20);
        float mipAngle = getAngle(point0,point17,point18);
        std::cout << "sumAngle :" << sumAngle << std::endl;
        std::cout << "mipAngle :" << mipAngle << std::endl;

        int fingerStatus;
        if (palmDirection == 0){
            if(point0ToPoint17 > point0ToPoint20 || mipAngle > 80 || sumAngle > 140){
                fingerStatus = 1;//close
            }else if(point0ToPoint17 * 1.1 < point0ToPoint20 && sumAngle > 20){
                fingerStatus = 2;//midclose
            }else if(point0ToPoint18 * 1.1 < point0ToPoint20 && sumAngle > 20){
                fingerStatus = 2;//midclose
            }else{
                fingerStatus = 0;//open
            }
	}else if(palmDirection == 2){
	    if(point0ToPoint17 > point0ToPoint20 || sumAngle > 80){
                fingerStatus = 1;//close
            }else if(mipAngle > 40){ 
		if(sumAngle < 35){
                    fingerStatus = 0;//open
		}else{
                    fingerStatus = 2;
                }
            }else if(mipAngle < 40){
		if(sumAngle < 25){
                    fingerStatus = 0;
		}else{
                    fingerStatus = 2;
                }
            }else{
                fingerStatus = 2;//midclose
	    }
	}
	return fingerStatus;
    }
};


REGISTER_CALCULATOR(HandGestureRecognitionCalculator);

::mediapipe::Status HandGestureRecognitionCalculator::GetContract(
    CalculatorContract *cc)
{
    RET_CHECK(cc->Inputs().HasTag(landmarkListTag));
    cc->Inputs().Tag(landmarkListTag).Set<mediapipe::LandmarkList>();

    RET_CHECK(cc->Inputs().HasTag(normRectTag));
    cc->Inputs().Tag(normRectTag).Set<NormalizedRect>();

    RET_CHECK(cc->Outputs().HasTag(recognizedHandGestureTag));
    cc->Outputs().Tag(recognizedHandGestureTag).Set<std::string>();

    return ::mediapipe::OkStatus();
}

::mediapipe::Status HandGestureRecognitionCalculator::Open(
    CalculatorContext *cc)
{
    cc->SetOffset(TimestampDiff(0));
    return ::mediapipe::OkStatus();
}

::mediapipe::Status HandGestureRecognitionCalculator::Process(
    CalculatorContext *cc)
{
    std::string *recognized_hand_gesture;
    recognized_hand_gesture = new std::string("___");

    const auto &landmarkList = cc->Inputs()
                                   .Tag(landmarkListTag)
                                   .Get<mediapipe::LandmarkList>();
    RET_CHECK_GT(landmarkList.landmark_size(), 0) << "Input landmark vector is empty.";

    // hand closed (red) rectangle
    const auto rect = &(cc->Inputs().Tag(normRectTag).Get<NormalizedRect>());
    const float rect_height = rect->height();
    const float rect_width = rect->width();
    const float rect_x_center = rect->x_center();
    const float rect_y_center = rect->y_center();
    const float rect_rotation = rect->rotation();

    // finger states
    int palmDirection = 0;
    int thumbIsOpen = 0;
    int firstFingerIsOpen = 0;
    int secondFingerIsOpen = 0;
    int thirdFingerIsOpen = 0;
    int fourthFingerIsOpen = 0;

    //palmDirection
    palmDirection = getPalmDirection(rect_rotation);
    std::cout << "palmDirection :" << palmDirection << std::endl;

    //thumbIsOpen
    thumbIsOpen = getThumbStatus(palmDirection,landmarkList.landmark(0),landmarkList.landmark(1),landmarkList.landmark(2),landmarkList.landmark(3),landmarkList.landmark(4),landmarkList.landmark(5),landmarkList.landmark(6),landmarkList.landmark(10),landmarkList.landmark(14),landmarkList.landmark(18));
    std::cout << "thumbIsOpen:"<< thumbIsOpen << std::endl;

    //firstFingerIsOpen
    firstFingerIsOpen = getFirstFingerStatus(palmDirection,landmarkList.landmark(2),landmarkList.landmark(3),landmarkList.landmark(5),landmarkList.landmark(6),landmarkList.landmark(7),landmarkList.landmark(8));
    std::cout << "firstFingerIsOpen:" << firstFingerIsOpen <<  std::endl;

    //secondFingerIsOpen
    secondFingerIsOpen = getSecondFingerStatus(palmDirection,landmarkList.landmark(0),landmarkList.landmark(9),landmarkList.landmark(10),landmarkList.landmark(11),landmarkList.landmark(12));
    std::cout << "secondFingerIsOpen:"<< secondFingerIsOpen  <<  std::endl;

    //thirdFingerIsOpen
    thirdFingerIsOpen = getThirdFingerStatus(palmDirection,landmarkList.landmark(0),landmarkList.landmark(13),landmarkList.landmark(14),landmarkList.landmark(15),landmarkList.landmark(16));
    std::cout << "thirdFingerIsOpen:"<< thirdFingerIsOpen  <<  std::endl;

    //fourthFingerIsOpen
    fourthFingerIsOpen = getFourthFingerStatus(palmDirection,landmarkList.landmark(0),landmarkList.landmark(17),landmarkList.landmark(18),landmarkList.landmark(19),landmarkList.landmark(20));
    std::cout << "fourthFingerIsOpen:"<< fourthFingerIsOpen <<  std::endl;

   // Hand gesture recognition
    recognized_hand_gesture = new std::string("___");

    if (rect_height > 0.2 && rect_height < 2){
	recognized_hand_gesture = new std::string(" ");
	if (palmDirection == 0 && thumbIsOpen==1 && firstFingerIsOpen==0 && secondFingerIsOpen==1 && thirdFingerIsOpen==1 && fourthFingerIsOpen==1){
            recognized_hand_gesture = new std::string("1");
        }else if (palmDirection == 0 && thumbIsOpen==1 && firstFingerIsOpen==0 && secondFingerIsOpen==0 && thirdFingerIsOpen==1 && fourthFingerIsOpen==1){
            recognized_hand_gesture = new std::string("2");
        }else if (palmDirection == 0 && thumbIsOpen==1 && firstFingerIsOpen==0 && secondFingerIsOpen==0 && thirdFingerIsOpen==0 && fourthFingerIsOpen==1){
            recognized_hand_gesture = new std::string("3");
        }else if (palmDirection == 0 && thumbIsOpen==1 && firstFingerIsOpen==0 && secondFingerIsOpen==0 && thirdFingerIsOpen==0 && fourthFingerIsOpen==0){
            recognized_hand_gesture = new std::string("4");
        }else if (palmDirection == 0 && thumbIsOpen==0 && firstFingerIsOpen==1 && secondFingerIsOpen==1 && thirdFingerIsOpen==1 && fourthFingerIsOpen==1){
            recognized_hand_gesture = new std::string("5");
        }else if (palmDirection == 2 && thumbIsOpen==0 && firstFingerIsOpen==0 && secondFingerIsOpen==1 && thirdFingerIsOpen==1 && fourthFingerIsOpen==1){
            recognized_hand_gesture = new std::string("6");
        }else if (palmDirection == 2 && thumbIsOpen==0 && firstFingerIsOpen==0 && secondFingerIsOpen==0 && thirdFingerIsOpen==1 && fourthFingerIsOpen==1){
            recognized_hand_gesture = new std::string("7");
        }else if (palmDirection == 2 && thumbIsOpen==0 && firstFingerIsOpen==0 && secondFingerIsOpen==0 && thirdFingerIsOpen==0 && fourthFingerIsOpen==1){
            recognized_hand_gesture = new std::string("8");
        }else if (palmDirection == 2 && thumbIsOpen==0 && firstFingerIsOpen==0 && secondFingerIsOpen==0 && thirdFingerIsOpen==0 && fourthFingerIsOpen==0){
            recognized_hand_gesture = new std::string("9");
        }else if (palmDirection == 0 && thumbIsOpen==1 && firstFingerIsOpen==2 && secondFingerIsOpen==1 && thirdFingerIsOpen==1 && fourthFingerIsOpen==1){
            recognized_hand_gesture = new std::string("10");
        }else if (palmDirection == 0 && thumbIsOpen==1 && firstFingerIsOpen==2 && secondFingerIsOpen==2 && thirdFingerIsOpen==1 && fourthFingerIsOpen==1){
            recognized_hand_gesture = new std::string("20");
        }else if (palmDirection == 0 && thumbIsOpen==1 && firstFingerIsOpen==2 && secondFingerIsOpen==2 && thirdFingerIsOpen==2 && fourthFingerIsOpen==1){
            recognized_hand_gesture = new std::string("30");
        }else if (palmDirection == 0 && thumbIsOpen==1 && firstFingerIsOpen==2 && secondFingerIsOpen==2 && thirdFingerIsOpen==2 && fourthFingerIsOpen==2){
            recognized_hand_gesture = new std::string("40");
        }else if (palmDirection == 2 && thumbIsOpen==2 && firstFingerIsOpen==1 && secondFingerIsOpen==1 && thirdFingerIsOpen==1 && fourthFingerIsOpen==1){
            recognized_hand_gesture = new std::string("50");
        }else if (palmDirection == 2 && thumbIsOpen==2 && firstFingerIsOpen==2 && secondFingerIsOpen==1 && thirdFingerIsOpen==1 && fourthFingerIsOpen==1){
            recognized_hand_gesture = new std::string("60");
        }else if (palmDirection == 2 && thumbIsOpen==2 && firstFingerIsOpen==2 && secondFingerIsOpen==2 && thirdFingerIsOpen==1 && fourthFingerIsOpen==1){
            recognized_hand_gesture = new std::string("70");
        }else if (palmDirection == 2 && thumbIsOpen==2 && firstFingerIsOpen==2 && secondFingerIsOpen==2 && thirdFingerIsOpen==2 && fourthFingerIsOpen==1){
            recognized_hand_gesture = new std::string("80");
        }else if (palmDirection == 2 && thumbIsOpen==2 && firstFingerIsOpen==2 && secondFingerIsOpen==2 && thirdFingerIsOpen==2 && fourthFingerIsOpen==2){
            recognized_hand_gesture = new std::string("90");
        }
    }

    cc->Outputs()
        .Tag(recognizedHandGestureTag)
        .Add(recognized_hand_gesture, cc->InputTimestamp());
    return ::mediapipe::OkStatus();
}

} // namespace mediapipe
