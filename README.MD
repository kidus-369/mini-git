# MiniGit

MiniGit is a lightweight version control system designed to provide basic functionality for managing file versions and tracking changes. It is implemented in C++ and uses SQLite for managing commit metadata and file snapshots.

## Features

- Initialize a new repository.
- Commit changes with a message and optional author.
- View commit history.
- Restore files from a specific commit.
- Save commit logs to a file.

## Commands

Below are the commands supported by MiniGit:

### 1. Initialize Repository

```bash
MiniGit init
```

- Initializes a new MiniGit repository by creating the necessary folder structure and database.

### 2. Commit Changes

```bash
MiniGit commit -m "<message>" [--author "<author>"]
```

- Saves a snapshot of the current files.
- **Options**:
  - `-m "<message>"`: Commit message (required).
  - `--author "<author>"`: Author name (optional).

### 3. View Commit History

```bash
MiniGit log
```

- Displays the commit history in the format:
  ```
  <commit_id> | <parent_id> | <author> | <timestamp> | <message>
  ```

### 4. Save Commit History to a File

```bash
MiniGit log --save <file>
```

- Saves the commit history to the specified file.

### 5. Restore Files from a Commit

```bash
MiniGit restore <commit_id>
```

- Restores files from the specified commit.

### 6. Help

```bash
MiniGit help
```

- Displays the help menu with a list of available commands.

## How It Works

1. **Initialization**:
   - Creates a `.mgit` folder to store repository data.
   - Sets up an SQLite database to manage commit metadata.

2. **Committing Changes**:
   - Generates a unique commit ID.
   - Saves a snapshot of the current files.
   - Stores commit metadata (message, author, timestamp) in the database.

3. **Viewing Logs**:
   - Retrieves commit history from the database and displays it in a readable format.

4. **Restoring Files**:
   - Restores the file structure and contents from a specific commit.

## Project Structure

- **`include/`**: Contains header files for various components (e.g., CLI, CommitHandler, FileSystem, Utils).
- **`src/`**: Contains the main implementation files.
- **`third_party/`**: Includes external dependencies like SQLite.

## Dependencies

- SQLite: Used for managing commit metadata.
- C++ Standard Library: Includes utilities like `filesystem`, `random`, and `chrono`.

## Build From Source

1. Clone the repository:
   ```bash
   git clone <repository-url>
   ```
2. Navigate to the project directory:
   ```bash
   cd mini-git
   ```
3. Build the project using CMake:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

## Usage

Run the `MiniGit` executable with the desired command. For example:

```bash
./MiniGit init
./MiniGit commit -m "Initial commit" --author "John Doe"
./MiniGit log
```
