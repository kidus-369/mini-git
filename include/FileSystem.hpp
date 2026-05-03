#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <string>
#include <filesystem>


// alias filesystem
namespace fs = std::filesystem;

namespace FileSystem {

    const std::string REPO_FOLDER = ".mgit";
    const std::string COMMITS_FOLDER = ".mgit/commits";

    enum class FileErrorType {
        None,
        NotFound,
        AlreadyExists,
        AccessDenied,
        InvalidName,
        DiskFull,
        Unknown
    };

    struct FileSystemError {
        FileErrorType type;
        std::string message;
        std::string path;
    };

    bool folder_already_exists(const std::string &folderName) {
        return fs::exists(folderName) && fs::is_directory(folderName);
    }

    FileSystemError* create_folder(const std::string &folderName) {
        FileSystemError* error = new FileSystemError; 
        error->path = folderName;
        error->type = FileErrorType::None; 

        if (folderName.empty()) {
            error->message = "Folder name is empty";
            error->type = FileErrorType::InvalidName;
            return error;
        }

        if (folder_already_exists(folderName)) { 
            error->message = "Folder already exists"; 
            error->type = FileErrorType::AlreadyExists;
            return error;
        }

        std::error_code ec;
        
        fs::create_directory(folderName, ec);

        // if there was an error while creatin the folder
        if (ec) {
            error->message = ec.message(); // Gets the system error message
            error->type = FileErrorType::AccessDenied; 
        }

        return error;
    }

    FileSystemError* initialize_folders() {
        FileSystemError* error = create_folder(REPO_FOLDER);

        if (error->type != FileErrorType::None) {
            return error; 
        }

        delete error; 
        error = create_folder(COMMITS_FOLDER);

        return error; 
    }
    FileSystemError* create_commit_directory(const std::string& commitID) {
    std::string path = COMMITS_FOLDER + "/" + commitID;
    return create_folder(path);
}
FileSystemError* copy_file(const fs::path& source, const fs::path& destination) {
    FileSystemError* error = new FileSystemError;
    error->type = FileErrorType::None;
    error->path = source.string();

    if (!fs::exists(source)) {
        error->type = FileErrorType::NotFound;
        error->message = "Source file not found";
        return error;
    }

    std::error_code ec;
    fs::copy_file(source, destination, fs::copy_options::overwrite_existing, ec);

    if (ec) {
        error->type = FileErrorType::AccessDenied;
        error->message = ec.message();
    }

    return error;
}
FileSystemError* snapshot(const std::string& commitID) {
  if (commitID.empty()) {
    FileSystemError* error = new FileSystemError;
    error->type = FileErrorType::InvalidName;
    error->message = "Commit ID is empty";
    error->path = "";
    return error;
    }
  if (!folder_already_exists(REPO_FOLDER)) {
    FileSystemError* error = new FileSystemError;
    error->type = FileErrorType::NotFound;
    error->message = "Repository not initialized";
    error->path = REPO_FOLDER;
    return error;
}
    FileSystemError* error = create_commit_directory(commitID);
    if (error->type != FileErrorType::None) {
    
        return error;
    }

    delete error;
    std::string commitPath = COMMITS_FOLDER + "/" + commitID;
    
  for (const auto& entry : fs::recursive_directory_iterator(".")) {

    std::string pathStr = entry.path().string();
  if (pathStr.find(REPO_FOLDER) != std::string::npos) {
      // if (entry.path().string().find(REPO_FOLDER) != std::string::npos) {
        continue;
    }
    fs::path relativePath = fs::relative(entry.path(), ".");
    fs::path destination = fs::path(commitPath) / relativePath;

    std::error_code ec;

    if (fs::is_directory(entry.path())) {
        fs::create_directories(destination, ec);

        if (ec) {
            FileSystemError* error = new FileSystemError;
            error->type = FileErrorType::AccessDenied;
            error->message = ec.message();
            error->path = destination.string();
            return error;
        }
    } 
    else {
        FileSystemError* copyErr = copy_file(entry.path(), destination);
        if (copyErr->type != FileErrorType::None) {
            return copyErr;
        }

        delete copyErr;
    }
}
   FileSystemError* success = new FileSystemError;
success->type = FileErrorType::None;
success->message = "Success";
success->path = "";
return success;
}

FileSystemError* restore(const std::string& commitID) {
       if (commitID.empty()) {
        FileSystemError* error = new FileSystemError;
        error->type = FileErrorType::InvalidName;
        error->message = "Commit ID is empty";
        error->path = "";
        return error;
    }

    if (!folder_already_exists(REPO_FOLDER)) {
    FileSystemError* error = new FileSystemError;
    error->type = FileErrorType::NotFound;
    error->message = "Repository not initialized";
    error->path = REPO_FOLDER;
    return error;
}

    std::string commitPath = COMMITS_FOLDER + "/" + commitID;

    if (!folder_already_exists(commitPath)) {
        FileSystemError* error = new FileSystemError;
        error->type = FileErrorType::NotFound;
        error->message = "Commit not found";
        error->path = commitPath;
        return error;
    }

  for (const auto& entry : fs::recursive_directory_iterator(commitPath)) {

    fs::path relativePath = fs::relative(entry.path(), commitPath);
    fs::path destination = fs::path("./") / relativePath;

    std::error_code ec;

    if (fs::is_directory(entry.path())) {
        fs::create_directories(destination, ec);

        if (ec) {
            FileSystemError* error = new FileSystemError;
            error->type = FileErrorType::AccessDenied;
            error->message = ec.message();
            error->path = destination.string();
            return error;
        }
    } else {
        FileSystemError* copyErr = copy_file(entry.path(), destination);

        if (copyErr->type != FileErrorType::None) {
            return copyErr;
        }

        delete copyErr;
    }
}

   FileSystemError* success = new FileSystemError;
success->type = FileErrorType::None;
success->message = "Success";
success->path = "";
return success;
}


}

#endif