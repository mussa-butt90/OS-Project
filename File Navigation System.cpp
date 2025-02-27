#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <limits> // Required for numeric_limits

// Forward declaration
class FileSystemNode;
class Directory;
class File;

// Enum to represent node type
enum NodeType { DIRECTORY_NODE, FILE_NODE };

// Base class for FileSystem nodes (Directories and Files)
class FileSystemNode {
public:
    std::string name;
    NodeType type;
    Directory* parent;

    FileSystemNode(std::string name, NodeType type, Directory* parent = nullptr) : name(name), type(type), parent(parent) {}

    virtual ~FileSystemNode() {} // Virtual destructor for proper cleanup

    virtual void listContents() {
        std::cout << name;
    }

    virtual bool isDirectory() const {
        return type == DIRECTORY_NODE;
    }
};

// Class to represent Directories
class Directory : public FileSystemNode {
public:
    std::vector<FileSystemNode*> children;

    Directory(std::string name, Directory* parent = nullptr) : FileSystemNode(name, DIRECTORY_NODE, parent) {}

    ~Directory() {
        // Recursively delete children to prevent memory leaks
        for (FileSystemNode* child : children) {
            delete child;
        }
    }

    void addChild(FileSystemNode* node) {
        children.push_back(node);
    }

    void removeChild(FileSystemNode* node) {
        for (size_t i = 0; i < children.size(); ++i) {
            if (children[i] == node) {
                children.erase(children.begin() + i);
                return;
            }
        }
    }

    FileSystemNode* findChild(const std::string& name) {
        for (FileSystemNode* child : children) {
            if (child->name == name) {
                return child;
            }
        }
        return nullptr;
    }

    void listContents() override {
        FileSystemNode::listContents(); // Call base class method to print directory name
        std::cout << ":" << std::endl;
        for (FileSystemNode* child : children) {
            std::cout << (child->isDirectory() ? "d " : "- ") << child->name << std::endl;
        }
    }
};

// Class to represent Files
class File : public FileSystemNode {
public:
    File(std::string name, Directory* parent) : FileSystemNode(name, FILE_NODE, parent) {}

    void listContents() override {
        FileSystemNode::listContents();
        std::cout << std::endl;
    }
};

// Function to get current working directory path
std::string getWorkingDirectoryPath(Directory* currentDir, Directory* rootDir) {
    if (currentDir == rootDir) {
        return "/";
    }
    std::string path = "";
    Directory* temp = currentDir;
    while (temp != rootDir) {
        path = "/" + temp->name + path;
        temp = temp->parent;
    }
    return path;
}

// Function to handle 'ls' command
void listDirectoryContents(Directory* currentDir) {
    currentDir->listContents();
}

// Function to handle 'cd' command
Directory* changeDirectory(Directory* currentDir, Directory* rootDir, const std::string& path) {
    if (path == "..") {
        if (currentDir->parent != nullptr) {
            return currentDir->parent;
        } else {
            std::cout << "Already at root directory." << std::endl;
            return currentDir;
        }
    } else if (path == "/") {
        return rootDir;
    } else {
        FileSystemNode* targetNode = currentDir->findChild(path);
        if (targetNode != nullptr && targetNode->isDirectory()) {
            return static_cast<Directory*>(targetNode);
        } else {
            std::cout << "cd: no such file or directory: " << path << std::endl;
            return currentDir;
        }
    }
}

// Function to handle 'mkdir' command
void makeDirectory(Directory* currentDir, const std::string& dirName) {
    if (currentDir->findChild(dirName) == nullptr) {
        Directory* newDir = new Directory(dirName, currentDir);
        currentDir->addChild(newDir);
    } else {
        std::cout << "mkdir: cannot create directory '" << dirName << "': File exists" << std::endl;
    }
}

// Function to handle 'touch' command
void createFile(Directory* currentDir, const std::string& fileName) {
    if (currentDir->findChild(fileName) == nullptr) {
        File* newFile = new File(fileName, currentDir);
        currentDir->addChild(newFile);
    } else {
        std::cout << "touch: cannot create file '" << fileName << "': File exists" << std::endl;
    }
}

// Function to handle 'rm' command
void removeFile(Directory* currentDir, const std::string& fileName) {
    FileSystemNode* fileToRemove = currentDir->findChild(fileName);
    if (fileToRemove != nullptr && !fileToRemove->isDirectory()) {
        currentDir->removeChild(fileToRemove);
        delete fileToRemove;
    } else if (fileToRemove == nullptr) {
        std::cout << "rm: cannot remove '" << fileName << "': No such file" << std::endl;
    }
    else {
        std::cout << "rm: cannot remove '" << fileName << "': is a directory" << std::endl;
    }
}

// Function to handle 'rmdir' command
void removeDirectory(Directory* currentDir, const std::string& dirName) {
    FileSystemNode* dirToRemove = currentDir->findChild(dirName);
    if (dirToRemove != nullptr && dirToRemove->isDirectory()) {
        Directory* directoryToDelete = static_cast<Directory*>(dirToRemove);
        if (directoryToDelete->children.empty()) {
            currentDir->removeChild(dirToRemove);
            delete dirToRemove;
        } else {
            std::cout << "rmdir: failed to remove '" << dirName << "': Directory not empty" << std::endl;
        }
    } else if (dirToRemove == nullptr) {
        std::cout << "rmdir: failed to remove '" << dirName << "': No such directory" << std::endl;
    }
    else {
        std::cout << "rmdir: failed to remove '" << dirName << "': Not a directory" << std::endl;
    }
}

int main() {
    Directory* rootDir = new Directory("/"); // Root directory
    Directory* currentDir = rootDir;

    std::string command;

    std::cout << "Welcome to Simple File System Navigation (Ubuntu Style)" << std::endl;
    std::cout << "Type 'help' to see available commands." << std::endl;

    while (true) {
        std::cout << "[user@ubuntu " << getWorkingDirectoryPath(currentDir, rootDir) << "]$ ";
        std::getline(std::cin, command);

        std::stringstream ss(command);
        std::string commandWord;
        std::string argument;

        ss >> commandWord;
        ss >> std::ws; // Consume leading whitespace before argument
        std::getline(ss, argument); // Read the rest of the line as argument

        if (commandWord == "help") {
            std::cout << "Available commands:" << std::endl;
            std::cout << "  ls       - list directory contents" << std::endl;
            std::cout << "  cd <dir> - change directory" << std::endl;
            std::cout << "  pwd      - print working directory" << std::endl;
            std::cout << "  mkdir <dir> - create directory" << std::endl;
            std::cout << "  touch <file> - create file" << std::endl;
            std::cout << "  rm <file>  - remove file" << std::endl;
            std::cout << "  rmdir <dir> - remove directory" << std::endl;
            std::cout << "  exit     - quit the program" << std::endl;
        } else if (commandWord == "ls") {
            listDirectoryContents(currentDir);
        } else if (commandWord == "cd") {
            currentDir = changeDirectory(currentDir, rootDir, argument);
        } else if (commandWord == "pwd") {
            std::cout << getWorkingDirectoryPath(currentDir, rootDir) << std::endl;
        } else if (commandWord == "mkdir") {
            makeDirectory(currentDir, argument);
        } else if (commandWord == "touch") {
            createFile(currentDir, argument);
        } else if (commandWord == "rm") {
            removeFile(currentDir, argument);
        } else if (commandWord == "rmdir") {
            removeDirectory(currentDir, argument);
        } else if (commandWord == "exit") {
            break;
        } else if (commandWord.empty()) {
            // Do nothing on empty input
        }
        else {
            std::cout << "Command not found: " << commandWord << std::endl;
            std::cout << "Type 'help' to see available commands." << std::endl;
        }
    }

    delete rootDir; // Clean up root directory and all its children (destructors will handle recursion)

    std::cout << "Exiting Simple File System Navigation." << std::endl;
    return 0;
}
