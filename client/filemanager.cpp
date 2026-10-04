/*
 * filemanager.cpp - C++ client for the Linux File Management Utility
 *
 * Demonstrates:
 *   - Linux system calls (mkdir, unlink, rename, opendir/readdir)
 *   - Writing to the filelogger kernel driver via /dev/filelogger
 *   - Object-Oriented C++ design (a FileManager class)
 *   - Basic Computer Architecture awareness (CPU info, user vs kernel space)
 *
 * Build : g++ -std=c++17 filemanager.cpp -o filemanager
 * Run   : ./filemanager     (driver must be loaded first)
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

constexpr const char* LOG_DEVICE = "/dev/filelogger";

/* ---------------- FileManager: wraps Linux file syscalls ---------------- */
class FileManager {
public:
    FileManager() {
        log_fd_ = open(LOG_DEVICE, O_WRONLY);
        if (log_fd_ < 0) {
            std::cerr << "Warning: could not open " << LOG_DEVICE
                      << " (driver not loaded?). Logging will be skipped.\n";
        }
    }

    ~FileManager() {
        if (log_fd_ >= 0) close(log_fd_);
    }

    void create_file(const std::string& filename) {
        std::ofstream file(filename);
        if (file) {
            std::cout << "[OK] File created: " << filename << "\n";
            log_to_driver("CREATE file: " + filename);
        } else {
            std::cout << "[ERROR] Could not create file: " << filename << "\n";
        }
    }

    void delete_file(const std::string& filename) {
        if (unlink(filename.c_str()) == 0) {
            std::cout << "[OK] File deleted: " << filename << "\n";
            log_to_driver("DELETE file: " + filename);
        } else {
            std::cout << "[ERROR] Could not delete file: " << filename
                      << " (" << strerror(errno) << ")\n";
        }
    }

    void rename_file(const std::string& oldname, const std::string& newname) {
        if (rename(oldname.c_str(), newname.c_str()) == 0) {
            std::cout << "[OK] Renamed " << oldname << " -> " << newname << "\n";
            log_to_driver("RENAME file: " + oldname + " -> " + newname);
        } else {
            std::cout << "[ERROR] Could not rename file: " << oldname
                      << " (" << strerror(errno) << ")\n";
        }
    }

    void make_directory(const std::string& dirname) {
        if (mkdir(dirname.c_str(), 0755) == 0) {
            std::cout << "[OK] Directory created: " << dirname << "\n";
            log_to_driver("MKDIR: " + dirname);
        } else {
            std::cout << "[ERROR] Could not create directory: " << dirname
                      << " (" << strerror(errno) << ")\n";
        }
    }

    void list_directory(const std::string& path) {
        DIR* dir = opendir(path.c_str());
        if (!dir) {
            std::cout << "[ERROR] Could not open directory: " << path << "\n";
            return;
        }
        std::cout << "--- Contents of " << path << " ---\n";
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::cout << "  " << entry->d_name << "\n";
        }
        closedir(dir);
        log_to_driver("LIST directory: " + path);
    }

    void view_file(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            std::cout << "[ERROR] Could not open file: " << filename << "\n";
            return;
        }
        std::cout << "--- Contents of " << filename << " ---\n";
        std::string line;
        while (std::getline(file, line)) {
            std::cout << line << "\n";
        }
        log_to_driver("VIEW file: " + filename);
    }

private:
    int log_fd_ = -1;

    void log_to_driver(const std::string& message) {
        if (log_fd_ < 0) return;
        write(log_fd_, message.c_str(), message.size());
    }
};

/* ---------------- Bonus: Computer Architecture info ---------------- */
void show_cpu_info() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (!cpuinfo.is_open()) {
        std::cerr << "Could not read /proc/cpuinfo\n";
        return;
    }
    std::string line;
    std::cout << "\n--- CPU Info (from /proc/cpuinfo) ---\n";
    while (std::getline(cpuinfo, line)) {
        if (line.rfind("model name", 0) == 0 ||
            line.rfind("cpu cores", 0) == 0) {
            std::cout << line << "\n";
        }
    }
}

void explain_architecture() {
    std::cout << "\n--- Kernel Space vs User Space ---\n"
              << "This C++ program runs in USER SPACE (limited privileges).\n"
              << "It cannot directly write to the kernel log buffer.\n"
              << "Instead, it opens /dev/filelogger and uses the write()\n"
              << "SYSTEM CALL, which crosses into KERNEL SPACE where the\n"
              << "filelogger driver records the message safely.\n";
}

/* ---------------- Menu-driven main ---------------- */
void print_menu() {
    std::cout << "\n=== Linux File Management Utility ===\n"
              << "1. Create a file\n"
              << "2. Delete a file\n"
              << "3. Rename a file\n"
              << "4. Create a directory\n"
              << "5. List a directory\n"
              << "6. View a file's contents\n"
              << "7. Show CPU info\n"
              << "8. Explain kernel vs user space\n"
              << "0. Exit\n"
              << "Choose: ";
}

int main() {
    FileManager fm;
    int choice = -1;

    while (choice != 0) {
        print_menu();
        std::cin >> choice;
        std::cin.ignore();

        std::string input1, input2;

        switch (choice) {
            case 1:
                std::cout << "Enter filename to create: ";
                std::getline(std::cin, input1);
                fm.create_file(input1);
                break;
            case 2:
                std::cout << "Enter filename to delete: ";
                std::getline(std::cin, input1);
                fm.delete_file(input1);
                break;
            case 3:
                std::cout << "Enter current filename: ";
                std::getline(std::cin, input1);
                std::cout << "Enter new filename: ";
                std::getline(std::cin, input2);
                fm.rename_file(input1, input2);
                break;
            case 4:
                std::cout << "Enter directory name to create: ";
                std::getline(std::cin, input1);
                fm.make_directory(input1);
                break;
            case 5:
                std::cout << "Enter directory path to list (e.g. .): ";
                std::getline(std::cin, input1);
                fm.list_directory(input1);
                break;
            case 6:
                std::cout << "Enter filename to view: ";
                std::getline(std::cin, input1);
                fm.view_file(input1);
                break;
            case 7:
                show_cpu_info();
                break;
            case 8:
                explain_architecture();
                break;
            case 0:
                std::cout << "Exiting...\n";
                break;
            default:
                std::cout << "Invalid choice.\n";
                break;
        }
    }
    return 0;
}
