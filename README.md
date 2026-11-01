# Student Record Management System

A menu-driven console application written in **C** that lets you add, view, search, update, delete and sort student records. All data is saved to a file, so it is still there the next time you run the program.

Built as a first-year B.Tech CSE college project.

## Features

- Add students (ID, name, age, gender, course, semester, phone, email, marks)
- Display all students in an aligned table
- Search by student ID or by name (partial, case-insensitive)
- Update any field (press Enter to keep the old value)
- Delete a student with a confirmation prompt
- Sort by ID or by name
- Average marks and highest-marks student
- Duplicate ID prevention
- Input validation for every field
- Safe file handling: missing, empty or corrupted data files do not crash the program

## Technologies Used

- C (C99)
- GCC / MinGW
- Visual Studio Code, Git and GitHub

## C Concepts Demonstrated

| Concept | Where it is used |
|---|---|
| Structures (`struct`, `typedef`) | `Student` and `FileHeader` |
| Arrays and strings | Fixed-size array of students, `char` arrays |
| Pointers | `findStudentById()` returns a pointer; pointer subtraction in delete; passing `int *count` |
| Functions / modular design | One function per feature |
| Function pointers | Validators passed into `promptText()` |
| File handling | `fopen`, `fread`, `fwrite`, `fclose`, `remove`, `rename` |
| Safe input | `fgets` + `strtol` / `strtod` instead of bare `scanf` |
| Error handling | Every file operation is checked; changes are undone if saving fails |

## Project Structure

```
student-record-management-system/
├── src/
│   └── main.c              # complete program
├── data/
│   └── .gitkeep            # keeps the empty folder in Git (students.dat is created here)
├── docs/
│   ├── TESTING.md          # test cases and expected results
│   └── GITHUB_GUIDE.md     # step-by-step upload guide
├── screenshots/
│   └── .gitkeep
├── .gitignore
├── LICENSE
└── README.md
```

## Installation and Compilation (Windows)

1. Install GCC through MinGW-w64 (for example via MSYS2) and check it works:
   ```
   gcc --version
   ```
2. Open a terminal **in the project's root folder** (the folder that contains `src` and `data`).
3. Compile:
   ```
   gcc -std=c99 -Wall -Wextra -o student_system.exe src/main.c
   ```
4. Run:
   ```
   .\student_system.exe
   ```

> Run it from the root folder. The program saves to the relative path `data/students.dat`.

### Using VS Code

1. `File > Open Folder...` and choose the project folder.
2. Open the terminal with `` Ctrl + ` ``. Make sure it is PowerShell or Command Prompt.
3. Run the compile and run commands above.

On Linux or macOS use `./student_system` instead of `.\student_system.exe`.

## Sample Menu Output

```
  ==============================================================
  ||                                                          ||
  ||            STUDENT RECORD MANAGEMENT SYSTEM              ||
  ||                 Built with C Programming                 ||
  ||                                                          ||
  ==============================================================

  ------------------------ MAIN MENU -------------------------
    1. Add Student
    2. Display All Students
    3. Search Student by ID
    4. Search Student by Name
    5. Update Student
    6. Delete Student
    7. Sort Students (by ID or Name)
    8. Show Average Marks
    9. Show Student with Highest Marks
    0. Save and Exit
  ------------------------------------------------------------
Enter your choice (0-9):
```

Sample table:

```
  ID     Name                   Age  Sex Course           Sem  Phone       Email                        Marks%
  ------ ---------------------- ---- --- ---------------- ---- ----------- ---------------------------- -------
  101    Asha Sharma            19   F   B.E. CSE         2    9876543210  asha@example.com             88.50
```

## Screenshots

Add your own screenshots to the `screenshots/` folder and link them here:

```
![Main menu](screenshots/main-menu.png)
![Student table](screenshots/student-table.png)
```

## Future Improvements

- Dynamic memory (`malloc` / `realloc`) so there is no fixed 500-record limit
- Password-protected login
- Export to CSV, import from CSV
- Search and filter by course or marks range
- Grades and rank calculation
- Split the code into multiple `.c` / `.h` files
- A text-based or SQLite storage backend

## Author

- **Name:** [Your Full Name]
- **GitHub:** [@your-github-username](https://github.com/your-github-username)

## License

Released under the [MIT License](LICENSE).
