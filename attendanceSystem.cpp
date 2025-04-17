#include <iostream>
#include <fstream>
#include <ctime>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#ifdef _WIN32
#include <conio.h>  
#else
#include <termios.h>  
#include <unistd.h>
#endif

using namespace std;
using namespace cv;


void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}


void returnToMenu() {
    cout << "Press any key to return to the menu...";
#ifdef _WIN32
    _getch();
#else
    getchar();
#endif
}

class AttendanceSystem {
public:
    void markStudentAttendance(const string& name, const string& rollNumber);
    void markTeacherAttendance(const string& name, const string& regNumber, int shiftNumber);
    void viewStudentAttendance() const;
    void viewTeacherAttendance() const;

private:
    void markAttendance(const string& name, const string& id, const string& logFileName) const;
    void displayAttendance(const string& logFileName) const;
};

void AttendanceSystem::markStudentAttendance(const string& name, const string& rollNumber) {
    markAttendance(name, rollNumber, "studentattendance_log.txt");
    clearScreen();
    cout << "Attendance marked for today: " << name << " " << rollNumber << endl;
    returnToMenu();
}

void AttendanceSystem::markTeacherAttendance(const string& name, const string& regNumber, int shiftNumber) {
    markAttendance(name, regNumber + " " + to_string(shiftNumber), "teacherattendance_log.txt");
    clearScreen();
    cout << "Attendance marked for today: " << name << " " << regNumber << " " << shiftNumber << endl;
    returnToMenu();
}

void AttendanceSystem::viewStudentAttendance() const {
    clearScreen();
    displayAttendance("studentattendance_log.txt");
    returnToMenu();
}

void AttendanceSystem::viewTeacherAttendance() const {
    clearScreen();
    displayAttendance("teacherattendance_log.txt");
    returnToMenu();
}

void AttendanceSystem::markAttendance(const string& name, const string& id, const string& logFileName) const {
    VideoCapture cap(0);

    if (!cap.isOpened()) {
        cerr << "Error opening video capture." << endl;
        return;
    }

    Mat frame;
    namedWindow("Face Detection", WINDOW_NORMAL);
    resizeWindow("Face Detection", 1280, 720);

    String face_cascade_path = "haarcascade_frontalface_default.xml";
    CascadeClassifier face_cascade;

    if (!face_cascade.load(face_cascade_path)) {
        cerr << "Error loading face cascade. Check the file path." << endl;
        return;
    }

    time_t start_time = time(0);
    while (time(0) - start_time < 3) {
        cap >> frame;

        if (frame.empty()) {
            cerr << "Error capturing frame." << endl;
            break;
        }

        Mat gray;
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        equalizeHist(gray, gray);

        vector<Rect> faces;
        face_cascade.detectMultiScale(gray, faces, 1.1, 3, 0, Size(30, 30));

        for (const Rect& face : faces) {
            rectangle(frame, face, Scalar(0, 255, 0), 2);

            Mat detectedFace = frame(face);
            string photoPath = "attendance_photos/" + name + "_" + id + ".jpg";
            imwrite(photoPath, detectedFace);
            cout << "Face image saved: " << photoPath << endl;
        }

        imshow("Face Detection", frame);

        if (waitKey(1) == 27) {
            destroyAllWindows();
            return;
        }
    }

    destroyAllWindows();

    time_t now = time(0);
    tm currentDate;
    localtime_s(&currentDate, &now);

    ofstream log(logFileName, ios::app);
    log << currentDate.tm_mday << "-" << currentDate.tm_mon + 1 << "-"
        << currentDate.tm_year + 1900 << " " << name << " " << id << " Photo: " << name + "_" + id + ".jpg" << endl;
    log.close();

    cout << "Attendance marked for today: " << name << " " << id << endl;
}

void AttendanceSystem::displayAttendance(const string& logFileName) const {
    ifstream log(logFileName);

    if (!log.is_open()) {
        cerr << "Error opening attendance log file." << endl;
        return;
    }

    cout << "Attendance Log:\n";
    string line;

    while (getline(log, line)) {
        cout << "\t" << line << endl;
    }

    log.close();
}

int main() {
    AttendanceSystem attendanceSystem;

    int choice;

    do {
        clearScreen();
        cout << endl;
        cout << endl;
        cout << "\t1) Mark Student Attendance\n";
        cout << "\t2) Mark Teacher Attendance\n";
        cout << "\t3) View Student Attendance\n";
        cout << "\t4) View Teacher Attendance\n";
        cout << "\t5) Exit\n";
        cout << "\tEnter your choice: ";
        cin >> choice;

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
        case 1: {
            string name, rollNumber;
            cout << endl;
            cout << "\tEnter Name: ";
            getline(cin, name);
            cout << "\tEnter Roll Number: ";
            getline(cin, rollNumber);
            attendanceSystem.markStudentAttendance(name, rollNumber);
            break;
        }
        case 2: {
            string name, regNumber;
            int shiftNumber;
            cout << endl;
            cout << "\tEnter Name: ";
            getline(cin, name);
            cout << "\tEnter Registration Number: ";
            getline(cin, regNumber);
            
            cout << "\tEnter Shift Number: ";
            cin >> shiftNumber;
            attendanceSystem.markTeacherAttendance(name, regNumber, shiftNumber);
            break;
        }
        case 3:
            attendanceSystem.viewStudentAttendance();
            break;
        case 4:
            attendanceSystem.viewTeacherAttendance();
            break;
        case 5:
            cout << "Exiting program.\n";
            break;
        default:
            cout << "Invalid choice. Please try again.\n";
        }

    } while (choice != 5);

    return 0;
}
