#include <iostream>
#include <string>
using namespace std;

class Elevator {
private:
    int floor;
    string status;

protected:
    int capacity;

public:
    Elevator(int f, int c) {
        floor = f;
        capacity = c;
        status = "Available";
    }

    int getFloor() {
        return floor;
    }

    string getStatus() {
        return status;
    }

    void setStatus(string s) {
        status = s;
    }

    void move(int f) {
        cout << "\nMoving from " << floor << " to " << f;
        floor = f;
    }

    void display() {
        cout << "\nFloor: " << floor
             << " | Status: " << status
             << " | Capacity: " << capacity;
    }
};


// final

class EmergencyElevator final : public Elevator {
public:
    EmergencyElevator(int f, int c) : Elevator(f, c) {}

    void emergencyMode() {
        setStatus("Emergency");
        cout << "\nEmergency mode activated.";
    }
};


class ElevatorRequest {
private:
    int source, destination;
    string status;

public:
    ElevatorRequest(int s, int d) {
        source = s;
        destination = d;
        status = "Waiting";
    }

    void setStatus(string s) {
        status = s;
    }

    int getSource() {
        return source;
    }

    int getDestination() {
        return destination;
    }

    // Stream operator overloading using friend
    friend ostream& operator<<(ostream& out,
                               const ElevatorRequest& r) {
        out << "\nRequest: " << r.source
            << " -> " << r.destination
            << " | " << r.status;
        return out;
    }
};


class ElevatorDispatchSystem {
private:
    Elevator* elevators[3];
    int count;

    // Static variable
    static int totalRequests;

    // Nested class
    class DispatchLog {
    public:
        void log(string msg) {
            cout << "\nLog: " << msg;
        }
    };

    DispatchLog log;

public:
    ElevatorDispatchSystem() {
        count = 0;
    }

    void addElevator(Elevator* e) {
        elevators[count++] = e;
        log.log("Elevator added.");
    }

    void assignRequest(ElevatorRequest& r) {

        int nearest = -1;
        int minDistance = 1000;

        for (int i = 0; i < count; i++) {

            if (elevators[i]->getStatus() == "Available") {

                int d = elevators[i]->getFloor()
                        - r.getSource();

                if (d < 0)
                    d = -d;

                if (d < minDistance) {
                    minDistance = d;
                    nearest = i;
                }
            }
        }

        if (nearest == -1) {
            cout << "\nNo elevator available.";
            return;
        }

        elevators[nearest]->setStatus("Busy");

        r.setStatus("Assigned");

        totalRequests++;

        elevators[nearest]->move(r.getSource());
        elevators[nearest]->move(r.getDestination());

        elevators[nearest]->setStatus("Available");
        r.setStatus("Completed");

        log.log("Request completed.");
    }

    // Static function
    static int getTotalRequests() {
        return totalRequests;
    }
};


// Static variable definition
int ElevatorDispatchSystem::totalRequests = 0;


int main() {

    Elevator e1(0, 8);
    Elevator e2(5, 10);
    EmergencyElevator e3(2, 8);

    ElevatorDispatchSystem system;

    system.addElevator(&e1);
    system.addElevator(&e2);
    system.addElevator(&e3);

    ElevatorRequest r1(3, 8);

    system.assignRequest(r1);

    // Stream operator
    cout << r1;

    // Object slicing
    EmergencyElevator e4(4, 8);
    Elevator e5 = e4;

    cout << "\n\nObject Slicing:";
    e5.display();

    // Static function
    cout << "\n\nTotal Requests: "
         << ElevatorDispatchSystem::getTotalRequests();

    return 0;
}