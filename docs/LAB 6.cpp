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

    friend bool operator==(const ElevatorRequest &r1, ElevatorRequest &r2); 

    friend void showRequest(const ElevatorRequest &r);
};
bool operator==(const ElevatorRequest &r1, ElevatorRequest &r2){
    return r1.sourceFloor == r2.sourceFloor && r1.destinationFloor==r2.destinationFloor;
}