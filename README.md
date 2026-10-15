# Elevator-Dispatching-System
A more practical and effecient take on Elevator Dispatch System
Made by
Shobhit Rai 20251502032
Krish Kumar 20251502015
Siddhi Vyas 20251501163
Tamanna Sharma 20251502034
Pranjali Nama 20251502022


# Elevator Dispatching System

A menu-driven C++ console project that simulates a multi-elevator building. It picks the best elevator for each request using a LOOK-style cost calculation and shows the main Object-Oriented Programming concepts in a practical example.

## Features

- 3 elevators in a building with floors 0 to 10
- Request an elevator by giving a source and a destination floor
- Smart dispatch: the elevator with the lowest cost is chosen
- Manual car controls: open door, close door, move to a floor
- Emergency handling with a dedicated emergency elevator and a maintenance rescue team
- Live status view of all elevators
- Input validation for floors, elevator numbers and menu choices

## OOP Concepts Used

| Concept | Where it is used |
|---|---|
| Classes and Objects | `Elevator`, `ElevatorRequest`, `MaintenanceStaff`, `ElevatorDispatchSystem` |
| Encapsulation | Private data with public getters and setters |
| Constructors and Destructor | All classes, virtual destructor in `Elevator` |
| Inheritance | `EmergencyElevator` inherits from `Elevator` |
| Polymorphism | Virtual functions `move()` and `display()` |
| Function Overriding | `display()` in `EmergencyElevator` |
| Nested Class | `ElevatorMotor` inside `Elevator` |
| Composition | `Elevator` has an `ElevatorMotor` |
| Aggregation | `ElevatorDispatchSystem` holds `Elevator*` pointers |
| Friend Function | `showRequestSummary()` |
| Operator Overloading | `operator==` and `operator<<` |
| Static Members | `totalRequests` and `getTotalRequests()` |
| Access Specifiers | `private`, `protected`, `public` |

## How the Dispatch Works

For every request, each elevator gets a cost score. The elevator with the lowest score is sent.

- Idle elevator: cost is the distance to the source floor
- Moving toward the passenger in the same direction: cost is the distance, which is the cheapest case
- Moving toward the passenger in the opposite direction: elevator must go to the end first, so a penalty of 2 is added
- Elevator has already passed the passenger: it must go to both ends and come back, so a penalty of 5 is added
- Elevators in Emergency or Maintenance status get a cost of 9999 and are never chosen

## Class Overview

| Class | Purpose |
|---|---|
| `Elevator` | Stores floor, direction, status, door state and capacity. Can move and open or close doors |
| `EmergencyElevator` | Special elevator with an emergency code and emergency mode |
| `ElevatorRequest` | Holds source floor, destination floor, direction and request status |
| `MaintenanceStaff` | Rescues passengers and restores an elevator after an emergency |
| `ElevatorDispatchSystem` | Manages all elevators, scores them and processes requests |

## Menu

```
1. Request Elevator
2. Elevator Car Controls
3. Report Emergency & Rescue
4. View Elevators Status
5. OOP Features & System Stats
6. Exit
```

## Getting Started

### Requirements

- A C++ compiler such as g++ (C++11 or newer)

### Compile and Run

```
g++ elevator.cpp -o elevator
./elevator
```

On Windows:

```
g++ elevator.cpp -o elevator.exe
elevator.exe
```

## Sample Output

Request from floor 1 to floor 5:

```
  [LOOK Algorithm Evaluation]
    - Elevator #1 @ F0 (IDLE) -> Cost: 1
    - Elevator #2 @ F6 (DOWN) -> Cost: 9
    - Elevator #3 @ F2 (IDLE) -> Cost: 1
  >>> Dispatching Elevator #1 to pickup.
  Elevator #1 moving UP [F0 -> F1] (Pickup)...
  Elevator #1 reached Floor 1.
  Elevator #1: Doors OPENED at Floor 1.
  Elevator #1: Doors CLOSED.
  Elevator #1 moving UP [F1 -> F5] (Dropoff)...
  Elevator #1 reached Floor 5.
  Elevator #1: Doors OPENED at Floor 5.
  Elevator #1: Doors CLOSED.
  [Request: F1 -> F5 (UP) | Completed]
```

When two elevators have the same cost, the one added first is chosen.

## Project Structure

```
Elevator-Dispatching-System/
├── docs/
│   └── elevator.cpp
└── README.md
```

## Future Improvements

- Handle multiple requests at the same time
- Add a graphical interface
- Add elevator load and weight limit checks
- Save request history to a file
