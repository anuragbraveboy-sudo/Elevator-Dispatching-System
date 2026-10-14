#include <iostream>
using namespace std;

class Elevator {
private:
    int floor;
    int capacity;

public:
    Elevator(int floor, int capacity) {
        this->floor = floor;
        this->capacity = capacity;
    }

    int getFloor() {
        return floor;
    }

    virtual void display() {
        cout << "Normal Elevator" << endl;
        cout << "Floor: " << floor << endl;
        cout << "Capacity: " << capacity << endl;
    }
};

class EmergencyElevator : public Elevator {
public:
    EmergencyElevator(int floor, int capacity)
        : Elevator(floor, capacity) {}

    void display() override {
        cout << "Emergency Elevator" << endl;
        cout << "Floor: " << getFloor() << endl;
    }
};

class SmartElevator : virtual public Elevator {
public:
    SmartElevator(int floor, int capacity)
        : Elevator(floor, capacity) {}
};

class MaintenanceElevator : virtual public Elevator {
public:
    MaintenanceElevator(int floor, int capacity)
        : Elevator(floor, capacity) {}
};

class MultiPurposeElevator
    : public SmartElevator,
      public MaintenanceElevator {

public:
    MultiPurposeElevator(int floor, int capacity)
        : Elevator(floor, capacity),
          SmartElevator(floor, capacity),
          MaintenanceElevator(floor, capacity) {}

    void show() {
        cout << "Multi-Purpose Elevator" << endl;
        cout << "Floor: " << getFloor() << endl;
    }
};

int main() {

    // Pointer to Object
    Elevator e1(3, 8);
    Elevator* ptr = &e1;
    ptr->display();

    cout << endl;

    // Pointer to Derived Class
    EmergencyElevator e2(5, 10);
    EmergencyElevator* emergencyPtr = &e2;
    emergencyPtr->display();

    cout << endl;

    // Virtual Function
    Elevator* basePtr = &e2;
    basePtr->display();

    cout << endl;

    // Virtual Base Class
    MultiPurposeElevator e3(7, 12);
    e3.show();

    return 0;
}