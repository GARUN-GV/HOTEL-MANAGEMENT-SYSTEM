#include <iostream>
#include <string>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

using namespace std;

// Establish Database Connection
sql::Connection* connectToDatabase() {
    try {
        sql::Driver* driver = get_driver_instance();
        sql::Connection* con = driver->connect("tcp://127.0.0.1:3306", "root", "J@ishriram63"); // Update credentials
        con->setSchema("HotelManagement");
        return con;
    }
    catch (sql::SQLException& e) {
        cerr << "Database connection error: " << e.what() << endl;
        system("pause");
        exit(EXIT_FAILURE);
    }
}

// Room Class
class Room {
public:
    int roomNumber;
    char ac, type, size;
    int rent, status;

    void addRoom();
    void removeRoom();
    void displayAvailableRooms();
    void searchRoom();
};

// Customer Class
class Customer {
public:
    string name, address, phone, fromDate, toDate;
    float paymentAdvance;
    int bookingID, roomNumber;

    void checkIn();
    void checkOut();
};

// Hotel Management Class
class HotelMgnt : public Room, public Customer {
public:
    void guestSummaryReport();
    void displayAllCustomers();  
};

// Add Room
void Room::addRoom() {
    try {
        sql::Connection* con = connectToDatabase();
        sql::PreparedStatement* pstmt = con->prepareStatement("INSERT INTO Rooms (RoomNumber, AC, Type, Size, Rent, Status) VALUES (?, ?, ?, ?, ?, ?)");

        cout << "Enter Room Number: ";
        cin >> roomNumber;
        cin.ignore(); // Clear buffer
        cout << "AC/Non-AC (A/N): ";
        cin >> ac;
        cout << "Comfort (S/N): ";
        cin >> type;
        cout << "Size (B/S): ";
        cin >> size;
        cout << "Daily Rent: ";
        cin >> rent;

        pstmt->setInt(1, roomNumber);
        pstmt->setString(2, string(1, ac));
        pstmt->setString(3, string(1, type));
        pstmt->setString(4, string(1, size));
        pstmt->setInt(5, rent);
        pstmt->setInt(6, 0); 

        pstmt->execute();
        cout << "Room added successfully!\n";

        delete pstmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        cerr << "Error adding room: " << e.what() << endl;
    }
}

// Display Available Rooms
void Room::displayAvailableRooms() {
    try {
        sql::Connection* con = connectToDatabase();
        sql::PreparedStatement* pstmt = con->prepareStatement("SELECT * FROM Rooms WHERE Status = 0");
        sql::ResultSet* res = pstmt->executeQuery();

        cout << "Available Rooms:\n";
        while (res->next()) {
            cout << "Room Number: " << res->getInt("RoomNumber") << "\n";
            cout << "AC/Non-AC: " << res->getString("AC") << "\n";
            cout << "Comfort: " << res->getString("Type") << "\n";
            cout << "Size: " << res->getString("Size") << "\n";
            cout << "Rent: " << res->getInt("Rent") << "\n";
            cout << "-------------------------\n";
        }

        delete res;
        delete pstmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        cerr << "Error fetching available rooms: " << e.what() << endl;
    }
}

// Search Room
void Room::searchRoom() {
    try {
        sql::Connection* con = connectToDatabase();
        sql::PreparedStatement* pstmt = con->prepareStatement("SELECT * FROM Rooms WHERE RoomNumber = ?");
        cout << "Enter Room Number: ";
        cin >> roomNumber;

        pstmt->setInt(1, roomNumber);
        sql::ResultSet* res = pstmt->executeQuery();

        if (res->next()) {
            cout << "Room Number: " << res->getInt("RoomNumber") << "\n";
            cout << "AC/Non-AC: " << res->getString("AC") << "\n";
            cout << "Comfort: " << res->getString("Type") << "\n";
            cout << "Size: " << res->getString("Size") << "\n";
            cout << "Rent: " << res->getInt("Rent") << "\n";
            cout << "Status: " << (res->getInt("Status") == 1 ? "Reserved" : "Available") << "\n";
        }
        else {
            cout << "Room not found!\n";
        }

        delete res;
        delete pstmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        cerr << "Error searching for room: " << e.what() << endl;
    }
}

// Check In
void Customer::checkIn() {
    try {
        sql::Connection* con = connectToDatabase();
        sql::PreparedStatement* pstmt = nullptr;

        cout << "Enter Room Number: ";
        cin >> roomNumber;

        pstmt = con->prepareStatement("SELECT Status FROM Rooms WHERE RoomNumber = ?");
        pstmt->setInt(1, roomNumber);
        sql::ResultSet* res = pstmt->executeQuery();

        if (res->next() && res->getInt("Status") == 1) {
            cout << "Room is already booked!\n";
        }
        else {
            delete pstmt;

            pstmt = con->prepareStatement("INSERT INTO Customers (Name, Address, Phone, FromDate, ToDate, PaymentAdvance, RoomNumber) VALUES (?, ?, ?, ?, ?, ?, ?)");

            cin.ignore();
            cout << "Enter Customer Name: ";
            getline(cin, name);
            cout << "Enter Address: ";
            getline(cin, address);
            cout << "Enter Phone: ";
            cin >> phone;
            cin.ignore();
            cout << "Enter From Date (YYYY-MM-DD): ";
            cin >> fromDate;
            cout << "Enter To Date (YYYY-MM-DD): ";
            cin >> toDate;
            cout << "Enter Advance Payment: ";
            cin >> paymentAdvance;

            pstmt->setString(1, name);
            pstmt->setString(2, address);
            pstmt->setString(3, phone);
            pstmt->setString(4, fromDate);
            pstmt->setString(5, toDate);
            pstmt->setDouble(6, paymentAdvance);
            pstmt->setInt(7, roomNumber);

            pstmt->execute();

            pstmt = con->prepareStatement("UPDATE Rooms SET Status = 1 WHERE RoomNumber = ?");
            pstmt->setInt(1, roomNumber);
            pstmt->execute();

            cout << "Customer checked in successfully!\n";
        }

        delete res;
        delete pstmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        cerr << "Error during check-in: " << e.what() << endl;
    }
}

// Check Out
void Customer::checkOut() {
    try {
        sql::Connection* con = connectToDatabase();
        sql::PreparedStatement* pstmt = nullptr;

        cout << "Enter Room Number: ";
        cin >> roomNumber;

        pstmt = con->prepareStatement("SELECT Rent, PaymentAdvance FROM Rooms INNER JOIN Customers ON Rooms.RoomNumber = Customers.RoomNumber WHERE Rooms.RoomNumber = ?");
        pstmt->setInt(1, roomNumber);
        sql::ResultSet* res = pstmt->executeQuery();

        if (res->next()) {
            float rent = res->getDouble("Rent");
            float advance = res->getDouble("PaymentAdvance");

            int days;
            cout << "Enter Number of Days Stayed: ";
            cin >> days;

            float totalBill = days * rent;
            cout << "Total Bill: " << totalBill << "\n";
            cout << "Advance Paid: " << advance << "\n";
            cout << "Remaining Amount: " << totalBill - advance << "\n";

            pstmt = con->prepareStatement("DELETE FROM Customers WHERE RoomNumber = ?");
            pstmt->setInt(1, roomNumber);
            pstmt->execute();

            pstmt = con->prepareStatement("UPDATE Rooms SET Status = 0 WHERE RoomNumber = ?");
            pstmt->setInt(1, roomNumber);
            pstmt->execute();

            cout << "Customer checked out successfully!\n";
        }
        else {
            cout << "Room not found or not booked!\n";
        }

        delete res;
        delete pstmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        cerr << "Error during check-out: " << e.what() << endl;
    }
}
// Remove Room
void Room::removeRoom() {
    try {
        sql::Connection* con = connectToDatabase();
        sql::PreparedStatement* pstmt = con->prepareStatement("SELECT Status FROM Rooms WHERE RoomNumber = ?");

        cout << "Enter Room Number to remove: ";
        cin >> roomNumber;

        pstmt->setInt(1, roomNumber);
        sql::ResultSet* res = pstmt->executeQuery();

        if (res->next()) {
            int status = res->getInt("Status");

            if (status == 1) {
                cout << "Cannot remove the room because it is currently booked.\n";
            }
            else {
                // Room is available, proceed to remove it
                delete pstmt;
                pstmt = con->prepareStatement("DELETE FROM Rooms WHERE RoomNumber = ?");
                pstmt->setInt(1, roomNumber);
                pstmt->execute();

                cout << "Room removed successfully!\n";
            }
        }
        else {
            cout << "Room not found!\n";
        }

        delete res;
        delete pstmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        cerr << "Error removing room: " << e.what() << endl;
    }
}


// Display All Customers
void HotelMgnt::displayAllCustomers() {
    try {
        sql::Connection* con = connectToDatabase();
        sql::PreparedStatement* pstmt = con->prepareStatement("SELECT * FROM Customers");
        sql::ResultSet* res = pstmt->executeQuery();

        cout << "All Customer Details:\n";
        while (res->next()) {
            cout << "Booking ID: " << res->getInt("BookingID") << "\n";
            cout << "Name: " << res->getString("Name") << "\n";
            cout << "Address: " << res->getString("Address") << "\n";
            cout << "Phone: " << res->getString("Phone") << "\n";
            cout << "From Date: " << res->getString("FromDate") << "\n";
            cout << "To Date: " << res->getString("ToDate") << "\n";
            cout << "Payment Advance: " << res->getDouble("PaymentAdvance") << "\n";
            cout << "Room Number: " << res->getInt("RoomNumber") << "\n";
            cout << "-------------------------\n";
        }

        delete res;
        delete pstmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        cerr << "Error fetching customer details: " << e.what() << endl;
    }
}

// Guest Summary Report
void HotelMgnt::guestSummaryReport() {
    try {
        sql::Connection* con = connectToDatabase();
        sql::PreparedStatement* pstmt = con->prepareStatement("SELECT * FROM Customers INNER JOIN Rooms ON Customers.RoomNumber = Rooms.RoomNumber WHERE Rooms.Status = 1");
        sql::ResultSet* res = pstmt->executeQuery();

        cout << "Guest Summary Report:\n";
        while (res->next()) {
            cout << "Booking ID: " << res->getInt("BookingID") << "\n";
            cout << "Name: " << res->getString("Name") << "\n";
            cout << "Room Number: " << res->getInt("RoomNumber") << "\n";
            cout << "Advance Paid: " << res->getDouble("PaymentAdvance") << "\n";
            cout << "From Date: " << res->getString("FromDate") << "\n";
            cout << "To Date: " << res->getString("ToDate") << "\n";
            cout << "-------------------------\n";
        }

        delete res;
        delete pstmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        cerr << "Error generating guest summary: " << e.what() << endl;
    }
}

// Main Menu
int main() {
    HotelMgnt hm;
    int choice;

    do {
        cout << "\nHotel Management System";
        cout << "\n1. Add Room";
        cout << "\n2. Check In";
        cout << "\n3. Check Out";
        cout << "\n4. Display Available Rooms";
        cout << "\n5. Search Room";
        cout << "\n6. Display All Customers";
        cout << "\n7. Remove rooms";
        cout << "\n8. Guest Summary Report";
        cout << "\n9. Exit";
        cout << "\nEnter your choice: ";
        cin >> choice;
        cout << endl;
        system("cls");

        switch (choice) {
           
        case 1:
            hm.addRoom();
            break;
        case 2:
            hm.checkIn();
            break;
        case 3:
            hm.checkOut();
            break;
        case 4:
            hm.displayAvailableRooms();
            break;
        case 5:
            hm.searchRoom();
            break;
        case 6:
            hm.displayAllCustomers();
            break;
        case 7:
            hm.removeRoom(); 
            break;
        case 8:
            hm.guestSummaryReport();
            break;
        case 9:
            cout << "Exiting...\n";
            break;
        default:
            cout << "Invalid choice! Please try again.\n";
        }
     
        cout << endl;
        cout << endl;
       
    } while (choice != 9);
    cout << endl << "Thank you for choosing Hotel Management System" << endl;
    return 0;
}

