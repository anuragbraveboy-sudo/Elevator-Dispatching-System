#include <iostream>
#include <string>
using namespace std;

class Passenger {

private:
    int currentFloor;
    int destinationFloor;

public:

    Passenger(int current, int destination) {
        currentFloor = current;
        destinationFloor = destination;
    }

    int getCurrentFloor() {
        return currentFloor;
    }

    int getDestinationFloor() {
        return destinationFloor;
    }

    void pressButton() {
        cout << "\nPassenger requested elevator from Floor "
             << currentFloor
             << " to Floor "
             << destinationFloor;
    }
};


class Elevator {

private:
    int currentFloor;
    string status;

protected:
    int capacity;

public:

    Elevator(int floor, int cap) {
        currentFloor = floor;
        capacity = cap;
        status = "Available";
    }

    int getFloor() {
        return currentFloor;
    }

    string getStatus() {
        return status;
    }

    void setStatus(string s) {
        status = s;
    }

    void move(int floor) {

        cout << "\nElevator moving from Floor "
             << currentFloor
             << " to Floor "
             << floor;

        currentFloor = floor;

        cout << "\nElevator reached Floor "
             << currentFloor;
    }

    void move(int floor, string reason) {

        cout << "\nReason: " << reason;

        move(floor);
    }

    void openDoor() {
        cout << "\nElevator door opened.";
    }

    void closeDoor() {
        cout << "\nElevator door closed.";
    }

    void display() {

        cout << "\n\nCurrent Floor : "
             << currentFloor;

        cout << "\nStatus        : "
             << status;

        cout << "\nCapacity      : "
             << capacity;
    }
};


class EmergencyElevator : public Elevator {

public:

    EmergencyElevator(int floor, int cap)
        : Elevator(floor, cap) {
    }

    void emergencyMode() {
        setStatus("Emergency");
        cout << "\nEmergency mode activated.";
    }
};


class ElevatorRequest {

private:
    int sourceFloor;
    int destinationFloor;
    string status;

public:

    ElevatorRequest(int source, int destination) {

        sourceFloor = source;
        destinationFloor = destination;
        status = "Waiting";
    }

    int getSourceFloor() {
        return sourceFloor;
    }

    int getDestinationFloor() {
        return destinationFloor;
    }

    string getStatus() {
        return status;
    }

    void setStatus(string s) {
        status = s;
    }

    void displayRequest() {

        cout << "\n\nSource Floor      : "
             << sourceFloor;

        cout << "\nDestination Floor : "
             << destinationFloor;

        cout << "\nStatus            : "
             << status;
    }

    bool operator==(const ElevatorRequest &r) {

        return sourceFloor == r.sourceFloor &&
               destinationFloor == r.destinationFloor;
    }

    friend void showRequest(const ElevatorRequest &r);
};


void showRequest(const ElevatorRequest &r) {

    cout << "\n\nFriend Function";

    cout << "\nRequest from Floor "
         << r.sourceFloor
         << " to Floor "
         << r.destinationFloor;
}


class MaintenanceStaff {

private:
    string staffName;
    bool available;

public:

    MaintenanceStaff(string name) {

        staffName = name;
        available = true;
    }

    void rescuePassenger(Elevator &e) {

        if (available) {

            cout << "\n\nMaintenance Team "
                 << staffName
                 << " is taking action.";

            cout << "\nPassenger is stuck inside the elevator.";

            e.setStatus("Under Maintenance");

            cout << "\nMaintenance team is rescuing the passenger.";

            cout << "\nPassenger rescued successfully.";

            e.setStatus("Available");

            cout << "\nElevator is now available.";
        }

        else {

            cout << "\nMaintenance team is unavailable.";
        }
    }
};


class Emergency {

private:
    int floor;
    string description;
    bool active;

public:

    Emergency(int f, string d) {

        floor = f;
        description = d;
        active = true;
    }

    void trigger() {

        cout << "\n\nElevator Emergency";

        cout << "\nFloor   : "
             << floor;

        cout << "\nProblem : "
             << description;

        cout << "\nEmergency reported.";
    }

    void resolve() {

        active = false;

        cout << "\nEmergency resolved.";
    }

    int getFloor() {
        return floor;
    }

    bool isActive() {
        return active;
    }
};


class ElevatorDispatchSystem {

private:

    Elevator *elevators[3];

    int numberOfElevators;

    static int totalRequests;

public:

    ElevatorDispatchSystem() {

        numberOfElevators = 0;
    }

    void addElevator(Elevator *e) {

        if (numberOfElevators < 3) {

            elevators[numberOfElevators] = e;

            numberOfElevators++;
        }
    }

    int findNearestElevator(int floor) {

        int nearest = -1;
        int minimumDistance = 1000;

        for (int i = 0; i < numberOfElevators; i++) {

            if (elevators[i]->getStatus() == "Available") {

                int distance =
                    elevators[i]->getFloor() - floor;

                if (distance < 0)
                    distance = -distance;

                if (distance < minimumDistance) {

                    minimumDistance = distance;
                    nearest = i;
                }
            }
        }

        return nearest;
    }

    void assignRequest(ElevatorRequest &request) {

        cout << "\n\nAssigning Elevator";

        int index =
            findNearestElevator(
                request.getSourceFloor()
            );

        if (index == -1) {

            cout << "\nNo elevator available.";

            return;
        }

        cout << "\nElevator selected.";

        elevators[index]->setStatus("Busy");

        request.setStatus("Assigned");

        totalRequests++;

        elevators[index]->move(
            request.getSourceFloor(),
            "Passenger Pickup"
        );

        elevators[index]->openDoor();

        cout << "\nPassenger enters.";

        elevators[index]->closeDoor();

        elevators[index]->move(
            request.getDestinationFloor(),
            "Passenger Destination"
        );

        elevators[index]->openDoor();

        cout << "\nPassenger exits.";

        elevators[index]->closeDoor();

        elevators[index]->setStatus("Available");

        request.setStatus("Completed");

        cout << "\nRequest completed.";
    }

    void handleEmergency(
        Emergency &emergency,
        MaintenanceStaff &staff) {

        cout << "\n\nEmergency Handling";

        emergency.trigger();

        int index =
            findNearestElevator(
                emergency.getFloor()
            );

        if (index == -1) {

            cout << "\nNo elevator available.";

            return;
        }

        cout << "\nNearest elevator found.";

        elevators[index]->setStatus("Emergency");

        elevators[index]->move(
            emergency.getFloor(),
            "Emergency Response"
        );

        staff.rescuePassenger(
            *elevators[index]
        );

        emergency.resolve();
    }

    void displaySystem() {

        cout << "\n\nElevator System";

        for (int i = 0; i < numberOfElevators; i++) {

            elevators[i]->display();
        }
    }

    static int getTotalRequests() {

        return totalRequests;
    }
};


int ElevatorDispatchSystem::totalRequests = 0;


int main() {

    cout << "Elevator Dispatch System\n";

    Passenger p1(3, 8);

    p1.pressButton();

    Elevator e1(0, 8);
    Elevator e2(5, 10);
    EmergencyElevator e3(2, 8);

    ElevatorDispatchSystem system;

    system.addElevator(&e1);
    system.addElevator(&e2);
    system.addElevator(&e3);

    system.displaySystem();

    ElevatorRequest request1(
        p1.getCurrentFloor(),
        p1.getDestinationFloor()
    );

    system.assignRequest(request1);

    ElevatorRequest request2(3, 8);

    cout << "\n\nOperator Overloading";

    if (request1 == request2)
        cout << "\nBoth requests are same.";
    else
        cout << "\nRequests are different.";

    showRequest(request1);

    MaintenanceStaff staff("Maintenance Team");

    Emergency emergency(
        6,
        "Passenger stuck inside elevator"
    );

    system.handleEmergency(
        emergency,
        staff
    );

    cout << "\n\nTotal Requests : "
         << ElevatorDispatchSystem::getTotalRequests();

    system.displaySystem();

    return 0;
}