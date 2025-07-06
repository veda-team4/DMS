#ifndef GLOBAL_H
#define GLOBAL_H

// ---------------------- 전역 상수 ----------------------
#define EAR_THRESH_VAL 0.30 // EAR 임계값
#define BLINK_RATIO_THRESH 0.6 // 눈 감은 비율 임계값
#define BLINK_WINDOW_MS 2000 // 분석 시간 윈도우 (2초)

#define INCREASE_THRESH = 2.0 // 고개 움직임 감지 최소 값
#define INCREASE_THRESH2 = 0.1 // 고개 움직임 미감지 최대 값 (이 값보다 적게 아래로 내려가면 떨어짐 아님)
#define INCREASE_TIME = 0.3 // 고개 움직임 감지 최대 시간 값
#define MAX_DOWN_COUNT = 5 // 몇번 카운트 해야 경고할 것인지
// --------------------------------------------------------

// ---------------------- 전역 변수 ----------------------
extern cv::VideoCapture cap;

extern dlib::shape_predictor sp; // 얼굴 랜드마크 추출하는 객체
extern dlib::rectangle biggestFaceRect; // 가장 큰 얼굴 사각형
extern bool hasFace; // 얼굴 감지 유무
extern std::mutex faceMutex; // 위 두 변수 위한 뮤텍스
extern cv::Mat sharedFrame; // 쓰레드에게 전달하는 프레임
extern std::mutex frameMutex; // 위 변수 위한 뮤텍스
extern std::atomic<bool> running; // 쓰레드 동작 제어 변수

extern std::vector<int> landmarkIdx; // 사용할 얼굴 랜드마크

extern int server_fd, client_fd; // 서버, 클라이언트 소켓 파일 디스크립터
// --------------------------------------------------------

#endif // GLOBAL_H