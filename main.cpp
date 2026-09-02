#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <sys/stat.h>
#include <unistd.h>

namespace fs = std::filesystem;

namespace {

bool isValidFileName(const std::string& name) {
    if (name.empty() || name.size() > 255) return false;
    if (name == "." || name == "..") return false;
    // Disallow path separators and control characters; prevent directory traversal.
    for (char c : name) {
        if (c == '/' || c == '\\' || c == '\0') return false;
        if (static_cast<unsigned char>(c) < 32) return false;
    }
    if (name.find("..") != std::string::npos) return false;
    return true;
}

bool isValidExtension(const std::string& ext) {
    if (ext.empty() || ext.size() > 32) return false;
    for (char c : ext) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-') return false;
    }
    return true;
}

std::string readToken(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::cin >> input;
    return input;
}

} // namespace

// Base class for file operations
class FileOperation {
public:
    virtual void execute() = 0;
    virtual ~FileOperation() = default;
};

class ListFilesOperation : public FileOperation {
public:
    void execute() override {
        std::cout << "Listing all files and directories in current directory:\n";
        for (const auto& entry : fs::directory_iterator(".")) {
            std::cout << "  " << entry.path().filename().string();
            if (entry.is_directory()) std::cout << "/";
            std::cout << "\n";
        }
    }
};

class CreateFileOperation : public FileOperation {
public:
    void execute() override {
        std::string fileName = readToken("Enter file name (without extension): ");
        std::string fileExtension = readToken("Enter file extension (e.g., txt, cpp): ");
        if (!isValidFileName(fileName) || !isValidExtension(fileExtension)) {
            throw std::runtime_error("Invalid file name or extension.");
        }
        fs::path filePath = fileName + "." + fileExtension;
        if (fs::exists(filePath)) {
            throw std::runtime_error("File already exists: " + filePath.string());
        }
        std::ofstream file(filePath);
        if (!file) {
            throw std::runtime_error("Error creating file: " + filePath.string());
        }
        std::cout << "File created successfully: " << filePath.string() << "\n";
    }
};

class DeleteFileOperation : public FileOperation {
public:
    void execute() override {
        std::string fileName = readToken("Enter file name to delete (including extension): ");
        if (!isValidFileName(fileName)) {
            throw std::runtime_error("Invalid file name.");
        }
        fs::path p(fileName);
        std::error_code ec;
        if (!fs::exists(p)) {
            throw std::runtime_error("File does not exist: " + fileName);
        }
        if (fs::is_directory(p)) {
            throw std::runtime_error("Refusing to delete directory with file delete: " + fileName);
        }
        if (!fs::remove(p, ec) || ec) {
            throw std::runtime_error("Error deleting file: " + ec.message());
        }
        std::cout << "File deleted successfully.\n";
    }
};

class RenameFileOperation : public FileOperation {
public:
    void execute() override {
        std::string oldFileName = readToken("Enter current file name (including extension): ");
        std::string newFileName = readToken("Enter new file name (including extension): ");
        if (!isValidFileName(oldFileName) || !isValidFileName(newFileName)) {
            throw std::runtime_error("Invalid file name.");
        }
        fs::path oldPath(oldFileName), newPath(newFileName);
        if (!fs::exists(oldPath)) {
            throw std::runtime_error("Source file does not exist: " + oldFileName);
        }
        if (fs::exists(newPath)) {
            throw std::runtime_error("Destination already exists: " + newFileName);
        }
        std::error_code ec;
        fs::rename(oldPath, newPath, ec);
        if (ec) {
            throw std::runtime_error("Error renaming file: " + ec.message());
        }
        std::cout << "File renamed successfully.\n";
    }
};

class EditFileOperation : public FileOperation {
public:
    void execute() override {
        std::string fileName = readToken("Enter file name to edit (including extension): ");
        if (!isValidFileName(fileName)) {
            throw std::runtime_error("Invalid file name.");
        }
        if (!fs::exists(fileName)) {
            throw std::runtime_error("File does not exist: " + fileName);
        }
        const char* editor = std::getenv("EDITOR");
        std::string editorCmd = editor ? editor : "nano";
        // Basic allowlist for editor to prevent injection; fallback to nano.
        // Only allow alphanumerics, dash, underscore, slash in editor path.
        for (char c : editorCmd) {
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '/' && c != '_' && c != '-' && c != '.') {
                editorCmd = "nano";
                break;
            }
        }
        std::string command = editorCmd + " \"" + fileName + "\"";
        int ret = std::system(command.c_str());
        if (ret != 0) {
            std::cout << "Editor exited with status " << ret << ". File may not have been edited.\n";
        }
    }
};

class SearchFileOperation : public FileOperation {
public:
    void execute() override {
        std::string fileName = readToken("Enter file name to search (including extension): ");
        if (!isValidFileName(fileName)) {
            throw std::runtime_error("Invalid file name.");
        }
        if (fs::exists(fs::path(fileName))) {
            std::cout << "File found: " << fileName << "\n";
        } else {
            std::cout << "File not found: " << fileName << "\n";
        }
    }
};

class FileDetailsOperation : public FileOperation {
public:
    void execute() override {
        std::string fileName = readToken("Enter file name to show details (including extension): ");
        if (!isValidFileName(fileName)) {
            throw std::runtime_error("Invalid file name.");
        }
        struct stat fileStat;
        if (stat(fileName.c_str(), &fileStat) == 0) {
            std::cout << "File Details:\n";
            std::cout << "  Name: " << fileName << "\n";
            std::cout << "  Size: " << fileStat.st_size << " bytes\n";
            std::cout << "  Owner UID: " << fileStat.st_uid << "\n";
            std::cout << "  Group GID: " << fileStat.st_gid << "\n";
            std::cout << "  Permissions: " << std::oct << (fileStat.st_mode & 0777) << std::dec << "\n";
            std::cout << "  Last modified: " << std::ctime(&fileStat.st_mtime);
        } else {
            throw std::runtime_error("Error retrieving file details: " + fileName);
        }
    }
};

class ViewFileOperation : public FileOperation {
public:
    void execute() override {
        std::string fileName = readToken("Enter file name to view (including extension): ");
        if (!isValidFileName(fileName)) {
            throw std::runtime_error("Invalid file name.");
        }
        std::ifstream file(fileName);
        if (file.is_open()) {
            std::cout << "File contents (" << fileName << "):\n";
            std::cout << "----------------------------------------\n";
            std::string line;
            while (std::getline(file, line)) {
                std::cout << line << "\n";
            }
            std::cout << "----------------------------------------\n";
        } else {
            throw std::runtime_error("Unable to open file: " + fileName);
        }
    }
};

class SortFileOperation : public FileOperation {
public:
    void execute() override {
        std::string fileName = readToken("Enter file name to sort (including extension): ");
        if (!isValidFileName(fileName)) {
            throw std::runtime_error("Invalid file name.");
        }
        std::ifstream file(fileName);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file: " + fileName);
        }
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) lines.push_back(line);
        file.close();
        std::sort(lines.begin(), lines.end());
        std::ofstream sortedFile(fileName);
        if (!sortedFile) throw std::runtime_error("Unable to write sorted file: " + fileName);
        for (const auto& l : lines) sortedFile << l << "\n";
        std::cout << "File sorted successfully (" << lines.size() << " lines).\n";
    }
};

class ListDirectoriesOperation : public FileOperation {
public:
    void execute() override {
        std::cout << "Listing all directories:\n";
        for (const auto& entry : fs::directory_iterator(".")) {
            if (entry.is_directory()) {
                std::cout << "  " << entry.path().filename().string() << "/\n";
            }
        }
    }
};

class ListFilesWithExtensionOperation : public FileOperation {
public:
    void execute() override {
        std::string extension = readToken("Enter file extension (e.g., txt, cpp): ");
        if (!isValidExtension(extension)) {
            throw std::runtime_error("Invalid extension.");
        }
        std::cout << "Listing all files with ." << extension << " extension:\n";
        bool found = false;
        for (const auto& entry : fs::directory_iterator(".")) {
            if (entry.is_regular_file() && entry.path().extension() == ("." + extension)) {
                std::cout << "  " << entry.path().filename().string() << "\n";
                found = true;
            }
        }
        if (!found) std::cout << "  (none)\n";
    }
};

class CountDirectoriesOperation : public FileOperation {
public:
    void execute() override {
        int count = 0;
        for (const auto& entry : fs::directory_iterator(".")) {
            if (entry.is_directory()) {
                std::string name = entry.path().filename().string();
                if (name != "." && name != "..") count++;
            }
        }
        std::cout << "Total number of directories: " << count << "\n";
    }
};

class CountFilesOperation : public FileOperation {
public:
    void execute() override {
        int count = 0;
        for (const auto& entry : fs::directory_iterator(".")) {
            if (entry.is_regular_file()) count++;
        }
        std::cout << "Total number of files: " << count << "\n";
    }
};

class SortFilesOperation : public FileOperation {
public:
    void execute() override {
        std::vector<std::string> files;
        for (const auto& entry : fs::directory_iterator(".")) {
            files.push_back(entry.path().filename().string());
        }
        std::sort(files.begin(), files.end());
        std::cout << "Sorted directory listing:\n";
        for (const auto& f : files) std::cout << "  " << f << "\n";
    }
};

void showMenu() {
    std::cout << "\n===== File Management Menu =====\n"
              << " 1. Show all files\n"
              << " 2. Create a new file\n"
              << " 3. Delete an existing file\n"
              << " 4. Rename a file\n"
              << " 5. Edit file content ($EDITOR or nano)\n"
              << " 6. Search for a file\n"
              << " 7. Show details of a file\n"
              << " 8. View contents of a file\n"
              << " 9. Sort contents of a file\n"
              << "10. List all directories\n"
              << "11. List files with a particular extension\n"
              << "12. Count total directories\n"
              << "13. Count total files\n"
              << "14. Sort files (directory listing)\n"
              << " 0. Exit\n";
}

void processChoice(int choice, const std::vector<std::unique_ptr<FileOperation>>& operations) {
    if (choice == 0) return;
    if (choice > 0 && choice <= static_cast<int>(operations.size())) {
        operations[static_cast<size_t>(choice - 1)]->execute();
    } else {
        throw std::runtime_error("Invalid choice. Please enter 0-" + std::to_string(operations.size()));
    }
}

int main() {
    std::vector<std::unique_ptr<FileOperation>> operations;
    operations.emplace_back(std::make_unique<ListFilesOperation>());
    operations.emplace_back(std::make_unique<CreateFileOperation>());
    operations.emplace_back(std::make_unique<DeleteFileOperation>());
    operations.emplace_back(std::make_unique<RenameFileOperation>());
    operations.emplace_back(std::make_unique<EditFileOperation>());
    operations.emplace_back(std::make_unique<SearchFileOperation>());
    operations.emplace_back(std::make_unique<FileDetailsOperation>());
    operations.emplace_back(std::make_unique<ViewFileOperation>());
    operations.emplace_back(std::make_unique<SortFileOperation>());
    operations.emplace_back(std::make_unique<ListDirectoriesOperation>());
    operations.emplace_back(std::make_unique<ListFilesWithExtensionOperation>());
    operations.emplace_back(std::make_unique<CountDirectoriesOperation>());
    operations.emplace_back(std::make_unique<CountFilesOperation>());
    operations.emplace_back(std::make_unique<SortFilesOperation>());

    while (true) {
        showMenu();
        std::cout << "Enter your choice (0 to exit): ";
        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cerr << "Error: Please enter a numeric choice.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 0) {
            std::cout << "Exiting. Goodbye!\n";
            break;
        }

        try {
            processChoice(choice, operations);
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Unexpected error: " << e.what() << "\n";
        }
    }

    return 0;
}
