#include <time.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <typeinfo>


#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/rect.pb.h"

namespace mediapipe
{

namespace
{
constexpr char recognizedHandGestureTag[] = "RECOGNIZED_HAND_GESTURE";
constexpr char assignmentPresentationTimeTag[] = "ASSIGNMENT_PRESENTATION_TIME";
constexpr char assignmentPresentationQuestionTag[] = "ASSIGNMENT_PRESENTATION_QUESTION";
constexpr char assignmentPresentationAnswerTag[] = "ASSIGNMENT_PRESENTATION_ANSWER";
constexpr char assignmentPresentationIsCorrectTag[] = "ASSIGNMENT_PRESENTATION_IS_CORRECT";
}

class AssignmentPresentationCalculator : public CalculatorBase
{
public:
    static ::mediapipe::Status GetContract(CalculatorContract *cc);
    ::mediapipe::Status Open(CalculatorContext *cc) override;
    ::mediapipe::Status Process(CalculatorContext *cc) override;

private:
    //past time
    time_t start_t = time(&start_t);
    time_t now_t;
    time_t end_t;
    time_t tmp_t;
    time_t hold_t = time(&hold_t);

    std::string getPastTime(){
	if (array_no <4){
            now_t = time(&now_t);
	    tmp_t = now_t;
        }else if (array_no == 4){
	    end_t = time(&now_t);
	    tmp_t = end_t;
	}else{
	    now_t = time(&now_t);
	    tmp_t = end_t;
	}
	double double_diff_time = difftime(tmp_t,start_t);
        int int_diff_time = double_diff_time;
        int past_time_min = int_diff_time / 60;
        int past_time_sec = int_diff_time % 60;
        std::string past_time = "time: " + std::to_string(past_time_min) + "min " + std::to_string(past_time_sec) + "sec";
        return past_time;
    }

    //QA and IsCorrect
    std::string array_question[5] = {"1+1 = ","1+2 = ","1+3 = ","1+4 = ","1+5 = "};
    std::string array_answer[5] = {"2","3","4","5","6"};
    int array_no = 0;

    std::string getQuestion(){
        if (array_no > 4){
            return std::string("FINISH");
	}else{
	    return std::string(array_question[array_no]);
	}
    }

    std::string getIsCorrect(std::string answer){
    	double double_diff_time = difftime(now_t,hold_t);
	if(double_diff_time < 1){
            return std::string("OK");
        }else if (array_no > 4){
	    return std::string("FINISH");
        }else if (answer == array_answer[array_no] ){
            hold_t = time(&now_t);
	    array_no ++ ;
            return std::string("OK");
        }else{
            return std::string("NG");	
        }
    }

};


REGISTER_CALCULATOR(AssignmentPresentationCalculator);

::mediapipe::Status AssignmentPresentationCalculator::GetContract(
    CalculatorContract *cc)
{
    RET_CHECK(cc->Inputs().HasTag(recognizedHandGestureTag));
    cc->Inputs().Tag(recognizedHandGestureTag).Set<std::string>();

    RET_CHECK(cc->Outputs().HasTag(assignmentPresentationTimeTag));
    cc->Outputs().Tag(assignmentPresentationTimeTag).Set<std::string>();

    RET_CHECK(cc->Outputs().HasTag(assignmentPresentationQuestionTag));
    cc->Outputs().Tag(assignmentPresentationQuestionTag).Set<std::string>();

    RET_CHECK(cc->Outputs().HasTag(assignmentPresentationAnswerTag));
    cc->Outputs().Tag(assignmentPresentationAnswerTag).Set<std::string>();

    RET_CHECK(cc->Outputs().HasTag(assignmentPresentationIsCorrectTag));
    cc->Outputs().Tag(assignmentPresentationIsCorrectTag).Set<std::string>();

    return ::mediapipe::OkStatus();
}

::mediapipe::Status AssignmentPresentationCalculator::Open(
    CalculatorContext *cc)
{
    cc->SetOffset(TimestampDiff(0));
    return ::mediapipe::OkStatus();
}

::mediapipe::Status AssignmentPresentationCalculator::Process(
    CalculatorContext *cc)
{
    std::string *assignment_presentation_time;
    std::string *assignment_presentation_question;
    std::string *assignment_presentation_answer;
    std::string *assignment_presentation_is_correct;
    std::string recognizedHandGesture  = cc->Inputs().Tag(recognizedHandGestureTag).Get<std::string>();

    std::string tmpPastTime = getPastTime();
    assignment_presentation_time = new std::string(tmpPastTime);

    //task
    std::string tmpQuestion = "question: " + getQuestion();
    //std::cout << tmpQuestion << std::endl;
    assignment_presentation_question = new std::string(tmpQuestion);
    
    std::string tmpAnswer = "answer: " + recognizedHandGesture;
    assignment_presentation_answer = new std::string(tmpAnswer);
    

    std::string tmpIsCorrect = getIsCorrect(recognizedHandGesture);
    assignment_presentation_is_correct = new std::string(tmpIsCorrect);

    cc->Outputs()
        .Tag(assignmentPresentationTimeTag)
        .Add(assignment_presentation_time, cc->InputTimestamp());

    cc->Outputs()
        .Tag(assignmentPresentationQuestionTag)
        .Add(assignment_presentation_question, cc->InputTimestamp());

    cc->Outputs()
        .Tag(assignmentPresentationAnswerTag)
        .Add(assignment_presentation_answer, cc->InputTimestamp());

    cc->Outputs()
        .Tag(assignmentPresentationIsCorrectTag)
        .Add(assignment_presentation_is_correct, cc->InputTimestamp());

	return ::mediapipe::OkStatus();
}

} // namespace mediapipe
