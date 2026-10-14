#include <iostream>
#include <string>
#include <vector>
#include <cmath>

using namespace std;

const int MIN_FLOOR = 0;
const int MAX_FLOOR = 10;

enum Direction { IDLE = 0, UP = 1, DOWN = 2 };

// Convert a direction into readable text.
string dirName(Direction direction) {
    if (direction == UP) {
        return "UP";
    }
    else if (direction == DOWN) {
        return "DOWN";
    }
    else {
        return "IDLE";
    }
}

class Elevator {
public:
    // Nested class representing the elevator motor.
    class ElevatorMotor {
    private:
        int hp;
        bool active;

    public:
        ElevatorMotor(int power = 75) {
            hp = power;
            active = false;
        }

        void start() {
            active = true;
        }

        void stop() {
            active = false;
        }
    };

private:
    int id;
    int currentFloor;
    Direction direction;
    string status;
    bool doorOpen;
    ElevatorMotor motor;

protected:
    int capacity;

public:
    // Constructor
    Elevator(int idNum, int floor, int cap) {
        id = idNum;
        currentFloor = floor;
        direction = IDLE;
        status = "Available";
        doorOpen = false;
        motor = ElevatorMotor(80);
        capacity = cap;
    }

    // Virtual destructor supports safe inheritance.
    virtual ~Elevator() {}

    // Getter functions
    int getId() const {
        return id;
    }

    int getFloor() const {
        return currentFloor;
    }

    Direction getDirection() const {
        return direction;
    }

    string getStatus() const {
        return status;
    }

    // Setter functions
    void setStatus(string newStatus) {
        status = newStatus;
    }

    void setDirection(Direction newDirection) {
        direction = newDirection;
    }

    void openDoor() {
        doorOpen = true;
        cout << "  Elevator #" << id
             << ": Doors OPENED at Floor " << currentFloor << ".\n";
    }

    void closeDoor() {
        doorOpen = false;
        cout << "  Elevator #" << id << ": Doors CLOSED.\n";
    }

    // Virtual function: a derived class can override this function.
    virtual void move(int targetFloor, const string& reason = "") {
        if (targetFloor == currentFloor) {
            return;
        }

        if (doorOpen == true) {
            closeDoor();
        }

        motor.start();

        if (targetFloor > currentFloor) {
            direction = UP;
        }
        else {
            direction = DOWN;
        }

        cout << "  Elevator #" << id << " moving " << dirName(direction);
        cout << " [F" << currentFloor << " -> F" << targetFloor << "]";

        if (reason != "") {
            cout << " (" << reason << ")";
        }

        cout << "...\n";

        currentFloor = targetFloor;
        direction = IDLE;
        motor.stop();

        cout << "  Elevator #" << id << " reached Floor "
             << currentFloor << ".\n";
    }

    virtual void display() const {
        cout << "  [Elevator " << id << "] Floor: " << currentFloor;
        cout << " | Dir: " << dirName(direction);
        cout << " | Status: " << status;

        if (doorOpen == true) {
            cout << " | Doors: OPEN";
        }
        else {
            cout << " | Doors: CLOSED";
        }

        cout << " | Cap: " << capacity << "\n";
    }

    // Friend function for stream output (cout << elevator).
    friend ostream& operator<<(ostream& output, const Elevator& elevator) {
        output << "[Elevator #" << elevator.id;
        output << " @ F" << elevator.currentFloor;
        output << " (" << dirName(elevator.direction) << ")";
        output << " | " << elevator.status << "]";
        return output;
    }
};

// Inheritance: EmergencyElevator inherits from Elevator.
class EmergencyElevator : public Elevator {
private:
    string code;

public:
    EmergencyElevator(int idNum, int floor, int cap, string siren = "EMG-911")
        : Elevator(idNum, floor, cap) {
        code = siren;
    }

    void triggerEmergencyMode() {
        setStatus("Emergency");
        setDirection(IDLE);

        cout << "  *** Emergency mode activated on Unit #" << getId();
        cout << " (Code: " << code << ") ***\n";
    }

    // Override the base class display function.
    void display() const override {
        Elevator::display();
        cout << "    `--> Priority Emergency Unit (" << code << ")\n";
    }
};

class ElevatorRequest {
private:
    int sourceFloor;
    int destinationFloor;
    Direction desiredDir;
    string status;

public:
    ElevatorRequest(int src, int dest) {
        sourceFloor = src;
        destinationFloor = dest;
        status = "Waiting";

        if (dest >= src) {
            desiredDir = UP;
        }
        else {
            desiredDir = DOWN;
        }
    }

    int getSourceFloor() const {
        return sourceFloor;
    }

    int getDestinationFloor() const {
        return destinationFloor;
    }

    Direction getDesiredDirection() const {
        return desiredDir;
    }

    void setStatus(string newStatus) {
        status = newStatus;
    }

    // Operator overloading: compare two requests using ==.
    bool operator==(const ElevatorRequest& other) const {
        if (sourceFloor == other.sourceFloor &&
            destinationFloor == other.destinationFloor) {
            return true;
        }

        return false;
    }

    // Operator overloading: print a request using cout << request.
    friend ostream& operator<<(ostream& output, const ElevatorRequest& request) {
        output << "[Request: F" << request.sourceFloor;
        output << " -> F" << request.destinationFloor;
        output << " (" << dirName(request.desiredDir) << ")";
        output << " | " << request.status << "]";
        return output;
    }

    // Friend function can access private request details.
    friend void showRequestSummary(const ElevatorRequest& request);
};

void showRequestSummary(const ElevatorRequest& request) {
    cout << "  (Friend Function) Request: F" << request.sourceFloor;
    cout << " -> F" << request.destinationFloor << "\n";
}

class MaintenanceStaff {
private:
    string name;

public:
    MaintenanceStaff(string staffName) {
        name = staffName;
    }

    void rescue(Elevator& elevator) {
        cout << "\n  [Maintenance: " << name << "]";
        cout << " Rescuing passengers from Elevator #" << elevator.getId() << "...\n";

        elevator.setStatus("Maintenance");
        elevator.openDoor();

        cout << "  [Maintenance] Passengers evacuated safely.\n";

        elevator.closeDoor();
        elevator.setStatus("Available");

        cout << "  [Maintenance] Elevator #" << elevator.getId();
        cout << " restored to Available.\n";
    }
};

class ElevatorDispatchSystem {
private:
    vector<Elevator*> elevators;
    static int totalRequests;

public:
    void addElevator(Elevator* elevator) {
        elevators.push_back(elevator);
    }

    static int getTotalRequests() {
        return totalRequests;
    }

    // Calculate a cost for choosing an elevator using LOOK-style logic.
    int calculateLookScore(const Elevator* elevator, int sourceFloor,
                           Direction userDirection) const {
        string elevatorStatus = elevator->getStatus();

        if (elevatorStatus != "Available" && elevatorStatus != "Busy") {
            return 9999;
        }

        int currentFloor = elevator->getFloor();
        Direction elevatorDirection = elevator->getDirection();

        // If the elevator is idle, use the distance to the request.
        if (elevatorDirection == IDLE) {
            return abs(currentFloor - sourceFloor);
        }

        if (elevatorDirection == UP) {
            // Elevator is going up and can pick up an upward-going passenger.
            if (sourceFloor >= currentFloor && userDirection == UP) {
                return sourceFloor - currentFloor;
            }

            // Elevator is going up, but passenger wants to go down.
            if (sourceFloor >= currentFloor && userDirection == DOWN) {
                return (MAX_FLOOR - currentFloor)
                     + (MAX_FLOOR - sourceFloor) + 2;
            }

            // Elevator has already passed the requested floor.
            return (MAX_FLOOR - currentFloor)
                 + (MAX_FLOOR - MIN_FLOOR)
                 + (sourceFloor - MIN_FLOOR) + 5;
        }

        if (elevatorDirection == DOWN) {
            // Elevator is going down and can pick up a downward-going passenger.
            if (sourceFloor <= currentFloor && userDirection == DOWN) {
                return currentFloor - sourceFloor;
            }

            // Elevator is going down, but passenger wants to go up.
            if (sourceFloor <= currentFloor && userDirection == UP) {
                return (currentFloor - MIN_FLOOR)
                     + (sourceFloor - MIN_FLOOR) + 2;
            }

            // Elevator has already passed the requested floor.
            return (currentFloor - MIN_FLOOR)
                 + (MAX_FLOOR - MIN_FLOOR)
                 + (MAX_FLOOR - sourceFloor) + 5;
        }

        return abs(currentFloor - sourceFloor) + 10;
    }

    int selectBestElevator(int sourceFloor, Direction userDirection) const {
        int bestIndex = -1;
        int lowestCost = 9999;

        cout << "\n  [LOOK Algorithm Evaluation]\n";

        for (int i = 0; i < (int)elevators.size(); i++) {
            int cost = calculateLookScore(elevators[i], sourceFloor, userDirection);

            cout << "    - Elevator #" << elevators[i]->getId();
            cout << " @ F" << elevators[i]->getFloor();
            cout << " (" << dirName(elevators[i]->getDirection()) << ")";
            cout << " -> Cost: " << cost << "\n";

            if (cost < lowestCost) {
                lowestCost = cost;
                bestIndex = i;
            }
        }

        if (lowestCost < 9000) {
            return bestIndex;
        }

        return -1;
    }

    void processRequest(ElevatorRequest& request) {
        int index = selectBestElevator(request.getSourceFloor(),
                                       request.getDesiredDirection());

        if (index == -1) {
            cout << "  No available elevator at the moment.\n";
            return;
        }

        Elevator* elevator = elevators[index];

        cout << "  >>> Dispatching Elevator #" << elevator->getId();
        cout << " to pickup.\n";

        elevator->setStatus("Busy");
        request.setStatus("In-Transit");
        totalRequests++;

        // Move to the passenger's source floor.
        elevator->move(request.getSourceFloor(), "Pickup");
        elevator->openDoor();
        elevator->closeDoor();

        // Move the passenger to the destination floor.
        elevator->move(request.getDestinationFloor(), "Dropoff");
        elevator->openDoor();
        elevator->closeDoor();

        elevator->setStatus("Available");
        request.setStatus("Completed");

        cout << "  " << request << "\n";
    }

    void handleEmergency(int floor, MaintenanceStaff& staff) {
        cout << "\n  [ALERT] Emergency reported at Floor " << floor << "!\n";

        int bestIndex = 0;
        int minimumDistance = 999;

        for (int i = 0; i < (int)elevators.size(); i++) {
            int distance = abs(elevators[i]->getFloor() - floor);

            if (distance < minimumDistance) {
                minimumDistance = distance;
                bestIndex = i;
            }
        }

        Elevator* elevator = elevators[bestIndex];
        elevator->setStatus("Emergency");
        elevator->move(floor, "Emergency Dispatch");
        staff.rescue(*elevator);
    }

    Elevator* getElevator(int index) {
        if (index >= 0 && index < (int)elevators.size()) {
            return elevators[index];
        }

        return nullptr;
    }

    int getCount() const {
        return (int)elevators.size();
    }

    void displayStatus() const {
        cout << "\n--- ELEVATOR FLEET STATUS ---\n";

        for (int i = 0; i < (int)elevators.size(); i++) {
            elevators[i]->display();
        }
    }
};

// Define and initialize the static variable.
int ElevatorDispatchSystem::totalRequests = 0;

int main() {
    // Create three elevators.
    Elevator elevator1(1, 0, 8);
    Elevator elevator2(2, 6, 10);
    EmergencyElevator elevator3(3, 2, 8, "RED-ALERT-911");

    elevator2.setDirection(DOWN);

    // Create the dispatch system and add the elevators.
    ElevatorDispatchSystem system;
    system.addElevator(&elevator1);
    system.addElevator(&elevator2);
    system.addElevator(&elevator3);

    MaintenanceStaff staff("Alpha Response Team");

    int choice = 0;

    while (true) {
        cout << "\n========== ELEVATOR CONTROL PANEL ==========\n";
        cout << "  [1] Request Elevator (Source & Destination)\n";
        cout << "  [2] Elevator Car Controls (Doors & Floor)\n";
        cout << "  [3] Report Emergency & Rescue\n";
        cout << "  [4] View Elevators Status\n";
        cout << "  [5] OOP Features & System Stats\n";
        cout << "  [6] Exit\n";
        cout << "Enter choice (1-6): ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }

        if (choice == 1) {
            int sourceFloor;
            int destinationFloor;

            cout << "Enter Source Floor (" << MIN_FLOOR << "-" << MAX_FLOOR << "): ";
            cin >> sourceFloor;

            cout << "Enter Destination Floor (" << MIN_FLOOR << "-" << MAX_FLOOR << "): ";
            cin >> destinationFloor;

            if (sourceFloor < MIN_FLOOR || sourceFloor > MAX_FLOOR ||
                destinationFloor < MIN_FLOOR || destinationFloor > MAX_FLOOR ||
                sourceFloor == destinationFloor) {
                cout << "Invalid floors entered!\n";
                continue;
            }

            ElevatorRequest request(sourceFloor, destinationFloor);
            system.processRequest(request);
        }
        else if (choice == 2) {
            system.displayStatus();

            cout << "Select Elevator (1-" << system.getCount() << "): ";

            int elevatorNumber;
            int action;
            cin >> elevatorNumber;

            // User enters 1, 2, or 3; vector indexes are 0, 1, or 2.
            Elevator* elevator = system.getElevator(elevatorNumber - 1);

            if (elevator == nullptr) {
                cout << "Invalid elevator!\n";
                continue;
            }

            cout << "Actions: [1] Open Door  [2] Close Door  [3] Move to Floor\n";
            cout << "Choice: ";
            cin >> action;

            if (action == 1) {
                elevator->openDoor();
            }
            else if (action == 2) {
                elevator->closeDoor();
            }
            else if (action == 3) {
                int targetFloor;

                cout << "Target Floor: ";
                cin >> targetFloor;

                if (targetFloor >= MIN_FLOOR && targetFloor <= MAX_FLOOR) {
                    elevator->move(targetFloor, "Manual Action");
                }
                else {
                    cout << "Invalid floor!\n";
                }
            }
        }
        else if (choice == 3) {
            int emergencyFloor;

            cout << "Emergency Floor (" << MIN_FLOOR << "-" << MAX_FLOOR << "): ";
            cin >> emergencyFloor;

            if (emergencyFloor < MIN_FLOOR || emergencyFloor > MAX_FLOOR) {
                cout << "Invalid floor!\n";
                continue;
            }

            elevator3.triggerEmergencyMode();
            system.handleEmergency(emergencyFloor, staff);
        }
        else if (choice == 4) {
            system.displayStatus();
        }
        else if (choice == 5) {
            cout << "\nTotal Requests Processed: ";
            cout << ElevatorDispatchSystem::getTotalRequests() << "\n";

            ElevatorRequest request1(1, 5);
            ElevatorRequest request2(1, 5);

            cout << "Stream Overloading: " << request1 << "\n";

            if (request1 == request2) {
                cout << "Equality Check (r1 == r2): MATCH\n";
            }
            else {
                cout << "Equality Check (r1 == r2): DIFFERENT\n";
            }

            showRequestSummary(request1);
        }
        else if (choice == 6) {
            cout << "Exiting Elevator System.\n";
            break;
        }
        else {
            cout << "Invalid choice. Please enter a number from 1 to 6.\n";
        }
    }

    return 0;
}