#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdexcept>

using namespace std;

// Base class for file operations
class FileOperation {
public:
    virtual void execute() = 0; // Pure virtual function
    virtual ~FileOperation() {} // Virtual destructor for polymorphic behavior
};

// Derived class for listing files
class ListFilesOperation : public FileOperation {
public:
    void execute() override {
#ifdef _WIN32
        cout << "Listing all files and directories..." << endl;
        system("dir");
#else
        cout << "Listing all files and directories..." << endl;
        system("ls");
#endif
    }
};

// Derived class for creating a file
class CreateFileOperation : public FileOperation {
public:
    void execute() override {
        string fileName, fileExtension;
        cout << "Enter the name of the file you want to create (without extension): ";
        cin >> fileName;
        cout << "Enter the file extension (e.g., txt, cpp): ";
        cin >> fileExtension;
        ofstream file(fileName + "." + fileExtension);
        if (!file) {
            throw runtime_error("Error creating file.");
        }
        cout << "File created successfully." << endl;
        file.close();
    }
};

// Derived class for deleting a file
class DeleteFileOperation : public FileOperation {
public:
    void execute() override {
#ifdef _WIN32
        string delCommand = "del ";
#else
        string delCommand = "rm ";
#endif
        string fileName;
        cout << "Enter the name of the file you want to delete (including extension): ";
        cin >> fileName;
        string command = delCommand + fileName;
        if (system(command.c_str()) == 0)
            cout << "File deleted successfully." << endl;
        else
            throw runtime_error("Error deleting file.");
    }
};

// Derived class for renaming a file
class RenameFileOperation : public FileOperation {
public:
    void execute() override {
        string oldFileName, newFileName;
        cout << "Enter the current name of the file (including extension): ";
        cin >> oldFileName;
        cout << "Enter the new name of the file (including extension): ";
        cin >> newFileName;
        if (rename(oldFileName.c_str(), newFileName.c_str()) == 0)
            cout << "File renamed successfully." << endl;
        else
            throw runtime_error("Error renaming file.");
    }
};

// Derived class for editing a file
class EditFileOperation : public FileOperation {
public:
    void execute() override {
        string fileName;
        cout << "Enter the name of the file you want to edit (including extension): ";
        cin >> fileName;
#ifdef _WIN32
        system(("notepad " + fileName).c_str());
#else
        system(("nano " + fileName).c_str());
#endif
    }
};

// Derived class for searching a file
class SearchFileOperation : public FileOperation {
public:
    void execute() override {
        string fileName;
        cout << "Enter the name of the file you want to search for (including extension): ";
        cin >> fileName;
        ifstream file(fileName);
        if (file.good())
            cout << "File found." << endl;
        else
            cout << "File not found." << endl;
    }
};

// Derived class for displaying file details
class FileDetailsOperation : public FileOperation {
public:
    void execute() override {
        string fileName;
        cout << "Enter the name of the file to show details (including extension): ";
        cin >> fileName;
        struct stat fileStat;
        if (stat(fileName.c_str(), &fileStat) == 0) {
            cout << "File Details:" << endl;
            cout << "Size: " << fileStat.st_size << " bytes" << endl;
            cout << "Owner UID: " << fileStat.st_uid << endl;
            cout << "Group GID: " << fileStat.st_gid << endl;
            cout << "Permissions: " << oct << fileStat.st_mode << endl;
            cout << "Last modified: " << ctime(&fileStat.st_mtime);
        }
        else
            throw runtime_error("Error retrieving file details.");
    }
};

// Derived class for viewing file contents
class ViewFileOperation : public FileOperation {
public:
    void execute() override {
        string fileName, line;
        cout << "Enter the name of the file you want to view (including extension): ";
        cin >> fileName;
        ifstream file(fileName);
        if (file.is_open()) {
            cout << "File contents:" << endl;
            while (getline(file, line))
                cout << line << endl;
            file.close();
        }
        else
            throw runtime_error("Unable to open file.");
    }
};

// Derived class for sorting file contents
class SortFileOperation : public FileOperation {
public:
    void execute() override {
        string fileName, line;
        vector<string> lines;
        cout << "Enter the name of the file you want to sort (including extension): ";
        cin >> fileName;
        ifstream file(fileName);
        if (file.is_open()) {
            while (getline(file, line))
                lines.push_back(line);
            file.close();
            sort(lines.begin(), lines.end());
            ofstream sortedFile(fileName);
            for (auto& l : lines)
                sortedFile << l << endl;
            sortedFile.close();
            cout << "File sorted successfully." << endl;
        }
        else
            throw runtime_error("Unable to open file.");
    }
};

// Derived class for listing directories
class ListDirectoriesOperation : public FileOperation {
public:
    void execute() override {
#ifdef _WIN32
        cout << "Listing all directories..." << endl;
        system("dir /AD");
#else
        cout << "Listing all directories..." << endl;
        system("ls -d */");
#endif
    }
};

// Derived class for listing files with a specific extension
class ListFilesWithExtensionOperation : public FileOperation {
public:
    void execute() override {
        string extension;
        cout << "Enter the file extension (e.g., txt, cpp): ";
        cin >> extension;
#ifdef _WIN32
        cout << "Listing all files with ." << extension << " extension..." << endl;
        system(("dir /B *." + extension).c_str());
#else
        cout << "Listing all files with ." << extension << " extension..." << endl;
        system(("ls *." + extension).c_str());
#endif
    }
};

// Derived class for counting directories
class CountDirectoriesOperation : public FileOperation {
public:
    void execute() override {
        DIR *dir;
        struct dirent *ent;
        int count = 0;
        if ((dir = opendir(".")) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                if (ent->d_type == DT_DIR) {
                    std::string name(ent->d_name);
                    if (name != "." && name != "..")
                        count++;
                }
            }
            closedir(dir);
            cout << "Total number of directories: " << count << endl;
        } else {
            throw runtime_error("Error counting directories.");
        }
    }
};

// Derived class for counting files
class CountFilesOperation : public FileOperation {
public:
    void execute() override {
        DIR *dir;
        struct dirent *ent;
        int count = 0;
        if ((dir = opendir(".")) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                if (ent->d_type == DT_REG) {
                    count++;
                }
            }
            closedir(dir);
            cout << "Total number of files: " << count << endl;
        } else {
            throw runtime_error("Error counting files.");
        }
    }
};

// Derived class for sorting files
class SortFilesOperation : public FileOperation {
public:
    void execute() override {
        cout << "Sorting files..." << endl;
#ifdef _WIN32
        cout << "This operation is not supported on Windows." << endl;
#else
        system("ls | sort");
#endif
    }
};

void showMenu();
void processChoice(int choice, vector<FileOperation*>& operations);

int main() {
    vector<FileOperation*> operations = {
        new ListFilesOperation(),
        new CreateFileOperation(),
        new DeleteFileOperation(),
        new RenameFileOperation(),
        new EditFileOperation(),
        new SearchFileOperation(),
        new FileDetailsOperation(),
        new ViewFileOperation(),
        new SortFileOperation(),
        new ListDirectoriesOperation(),
        new ListFilesWithExtensionOperation(),
        new CountDirectoriesOperation(),
        new CountFilesOperation(),
        new SortFilesOperation()
    };

    int choice;
    do {
        showMenu();
        cout << "Enter your choice (0 to exit): ";
        cin >> choice;
        cin.ignore(); 

        try {
            processChoice(choice, operations);
        } catch (const runtime_error& e) {
            cerr << "Error: " << e.what() << endl;
        }

    } while(choice != 0);

    // Cleanup
    for (auto op : operations) {
        delete op;
    }

    return 0;
}

void showMenu() {
    cout << "===== File Management Menu =====" << endl;
    cout << "1. Show all files" << endl;
    cout << "2. Create a new file" << endl;
    cout << "3. Delete an existing file" << endl;
    cout << "4. Rename a file" << endl;
    cout << "5. Edit file content" << endl;
    cout << "6. Search for a file" << endl;
    cout << "7. Show details of a file" << endl;
    cout << "8. View contents of a file" << endl;
    cout << "9. Sort contents of a file" << endl;
    cout << "10. List all directories" << endl;
    cout << "11. List files with a particular extension" << endl;
    cout << "12. Count total directories" << endl;
    cout << "13. Count total files" << endl;
    cout << "14. Sort files" << endl;
    cout << "0. Exit" << endl;
}

void processChoice(int choice, vector<FileOperation*>& operations) {
    if (choice >= 0 && choice < operations.size()) {
        operations[choice - 1]->execute();
    }
    else {
        throw runtime_error("Invalid choice. Please try again.");
    }
}