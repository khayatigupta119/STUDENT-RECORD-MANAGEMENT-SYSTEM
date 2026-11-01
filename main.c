/*
 * ============================================================================
 *  Student Record Management System
 *  Language : C (C99)
 *  Compiler : GCC / MinGW (Windows), also works on Linux and macOS
 *
 *  A menu-driven console application that stores student records in a binary
 *  file so that the data is still there after the program is closed.
 *
 *  Concepts used: struct, arrays, strings, pointers, functions, file handling
 *  (fopen/fread/fwrite/fclose), input validation and error handling.
 *
 *  IMPORTANT: run the program from the project's root folder, so that the
 *  relative path "data/students.dat" can be found.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/* ------------------------------ Constants -------------------------------- */

#define MAX_STUDENTS   500          /* maximum records the program can hold   */
#define NAME_LEN       50           /* includes the '\0' at the end           */
#define COURSE_LEN     30
#define PHONE_LEN      11           /* 10 digits + '\0'                       */
#define EMAIL_LEN      50
#define INPUT_LEN      128          /* size of the general input buffer       */

#define DATA_FILE      "data/students.dat"
#define TEMP_FILE      "data/students.tmp"
#define BACKUP_FILE    "data/students.corrupt.bak"
#define FILE_MAGIC     "SRM1"       /* marks a file as ours (4 characters)    */

/* ------------------------------ Data types ------------------------------- */

/* One student record. */
typedef struct {
    int   id;
    char  name[NAME_LEN];
    int   age;
    char  gender;                   /* 'M', 'F' or 'O'                        */
    char  course[COURSE_LEN];
    int   semester;
    char  phone[PHONE_LEN];
    char  email[EMAIL_LEN];
    float marks;                    /* percentage, 0 - 100                    */
} Student;

/* Small header written at the start of the data file. It lets us detect a
 * corrupted file or a file that is not ours. */
typedef struct {
    char magic[4];
    int  count;
} FileHeader;

/* ------------------------- Function declarations ------------------------- */

/* Input helpers */
static int  readLine(const char *prompt, char *buffer, size_t size);
static void trimSpaces(char *text);
static int  promptInt(const char *label, int min, int max, int *result, int allowBlank);
static int  promptFloat(const char *label, float min, float max, float *result, int allowBlank);
static int  promptText(const char *label, char *result, size_t size, int allowBlank,
                       int (*isValid)(const char *), const char *errorMessage);
static int  promptGender(char *result, int allowBlank);
static int  confirm(const char *question);
static void pauseScreen(void);

/* Validators (return 1 if the text is acceptable, otherwise 0) */
static int  isValidName(const char *text);
static int  isValidCourse(const char *text);
static int  isValidPhone(const char *text);
static int  isValidEmail(const char *text);

/* String helpers */
static int  compareIgnoreCase(const char *a, const char *b);
static int  containsIgnoreCase(const char *text, const char *part);

/* Display helpers */
static void printBanner(void);
static void printMenu(void);
static void printSectionTitle(const char *title);
static void printTableHeader(void);
static void printTableRow(const Student *s);
static void printStudentDetails(const Student *s);
static void printSuccess(const char *message);
static void printError(const char *message);

/* File handling */
static int  loadStudents(Student students[], int *count);
static int  saveStudents(const Student students[], int count);

/* Core features */
static Student *findStudentById(Student students[], int count, int id);
static void addStudent(Student students[], int *count);
static void displayAllStudents(const Student students[], int count);
static void searchById(Student students[], int count);
static void searchByName(const Student students[], int count);
static void updateStudent(Student students[], int count);
static void deleteStudent(Student students[], int *count);
static void sortStudents(Student students[], int count);
static void showAverageMarks(const Student students[], int count);
static void showTopStudent(const Student students[], int count);

/* ================================= main ================================== */

int main(void)
{
    Student students[MAX_STUDENTS];   /* all records live in this array       */
    int     count = 0;                /* how many records are in use          */
    int     choice;
    int     running = 1;

    printBanner();
    loadStudents(students, &count);   /* a missing file is not an error       */
    pauseScreen();

    while (running) {
        printMenu();
        if (!promptInt("Enter your choice (0-9): ", 0, 9, &choice, 0)) {
            continue;
        }

        switch (choice) {
            case 1: addStudent(students, &count);           break;
            case 2: displayAllStudents(students, count);    break;
            case 3: searchById(students, count);            break;
            case 4: searchByName(students, count);          break;
            case 5: updateStudent(students, count);         break;
            case 6: deleteStudent(students, &count);        break;
            case 7: sortStudents(students, count);          break;
            case 8: showAverageMarks(students, count);      break;
            case 9: showTopStudent(students, count);        break;
            case 0:
                printf("\n");
                printSuccess("All records are saved. Thank you for using the system. Goodbye!");
                running = 0;
                break;
            default: break;           /* cannot happen: range is checked      */
        }

        if (running) {
            pauseScreen();
        }
    }

    return 0;
}

/* ============================ Input helpers ============================== */

/*
 * Reads one line from the keyboard safely using fgets.
 *  - Removes the trailing newline.
 *  - If the user typed more than the buffer can hold, the extra characters
 *    are thrown away (so they do not leak into the next question) and the
 *    function returns 0.
 *  - Returns 1 on success.
 */
static int readLine(const char *prompt, char *buffer, size_t size)
{
    size_t len;
    int    ch;

    printf("%s", prompt);

    if (fgets(buffer, (int)size, stdin) == NULL) {
        /* Input stream closed (Ctrl+Z / Ctrl+D). Exit cleanly: data is
         * already saved after every change. */
        printf("\n\nInput closed. Exiting program.\n");
        exit(EXIT_SUCCESS);
    }

    len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';                    /* normal case             */
    } else {
        /* No newline found: the line was longer than the buffer. */
        while ((ch = getchar()) != '\n' && ch != EOF) {
            /* discard the rest of the line */
        }
        buffer[0] = '\0';
        return 0;
    }

    trimSpaces(buffer);
    return 1;
}

/* Removes leading/trailing spaces, tabs and '\r' (Windows line endings). */
static void trimSpaces(char *text)
{
    size_t start = 0;
    size_t len   = strlen(text);

    while (len > 0 && isspace((unsigned char)text[len - 1])) {
        text[--len] = '\0';
    }
    while (text[start] != '\0' && isspace((unsigned char)text[start])) {
        start++;
    }
    if (start > 0) {
        memmove(text, text + start, len - start + 1);
    }
}

/*
 * Asks for a whole number between min and max.
 * Return value: 1 = a valid number was stored in *result
 *               0 = user pressed Enter on a blank-allowed question (keep old)
 * Keeps asking until the input is valid. (For the menu, allowBlank is 0.)
 */
static int promptInt(const char *label, int min, int max, int *result, int allowBlank)
{
    char  input[INPUT_LEN];
    char *end;
    long  value;

    while (1) {
        if (!readLine(label, input, sizeof(input))) {
            printError("Input is too long. Please try again.");
            continue;
        }
        if (input[0] == '\0') {
            if (allowBlank) {
                return 0;
            }
            printError("This field cannot be empty.");
            continue;
        }

        errno = 0;
        value = strtol(input, &end, 10);
        if (*end != '\0' || errno == ERANGE) {
            printError("Please enter a valid whole number.");
            continue;
        }
        if (value < min || value > max) {
            printf("  [ERROR] Value must be between %d and %d.\n", min, max);
            continue;
        }

        *result = (int)value;
        return 1;
    }
}

/* Same idea as promptInt, but for decimal numbers (used for marks). */
static int promptFloat(const char *label, float min, float max, float *result, int allowBlank)
{
    char   input[INPUT_LEN];
    char  *end;
    double value;

    while (1) {
        if (!readLine(label, input, sizeof(input))) {
            printError("Input is too long. Please try again.");
            continue;
        }
        if (input[0] == '\0') {
            if (allowBlank) {
                return 0;
            }
            printError("This field cannot be empty.");
            continue;
        }

        errno = 0;
        value = strtod(input, &end);
        if (*end != '\0' || errno == ERANGE) {
            printError("Please enter a valid number (example: 78.5).");
            continue;
        }
        if (value < min || value > max) {
            printf("  [ERROR] Value must be between %.0f and %.0f.\n", min, max);
            continue;
        }

        *result = (float)value;
        return 1;
    }
}

/*
 * Asks for a piece of text and checks it with the validator function that is
 * passed in (isValid is a "pointer to a function").
 * Returns 1 if a valid value was stored, 0 if the user left it blank and
 * blank is allowed.
 */
static int promptText(const char *label, char *result, size_t size, int allowBlank,
                      int (*isValid)(const char *), const char *errorMessage)
{
    char input[INPUT_LEN];

    while (1) {
        if (!readLine(label, input, sizeof(input))) {
            printError("Input is too long. Please try again.");
            continue;
        }
        if (input[0] == '\0') {
            if (allowBlank) {
                return 0;
            }
            printError("This field cannot be empty.");
            continue;
        }
        if (strlen(input) >= size) {
            printf("  [ERROR] Too long. Maximum %d characters.\n", (int)size - 1);
            continue;
        }
        if (!isValid(input)) {
            printError(errorMessage);
            continue;
        }

        strcpy(result, input);        /* safe: length was checked above       */
        return 1;
    }
}

/* Asks for gender: M, F or O (Other). Stored as an uppercase character. */
static int promptGender(char *result, int allowBlank)
{
    char input[INPUT_LEN];

    while (1) {
        if (!readLine("  Gender (M/F/O)          : ", input, sizeof(input))) {
            printError("Input is too long. Please try again.");
            continue;
        }
        if (input[0] == '\0') {
            if (allowBlank) {
                return 0;
            }
            printError("This field cannot be empty.");
            continue;
        }
        if (input[1] == '\0' && strchr("MFOmfo", input[0]) != NULL) {
            *result = (char)toupper((unsigned char)input[0]);
            return 1;
        }
        printError("Enter M (Male), F (Female) or O (Other).");
    }
}

/* Asks a yes/no question. Returns 1 for yes, 0 for no. */
static int confirm(const char *question)
{
    char input[INPUT_LEN];

    while (1) {
        printf("%s (y/n): ", question);
        if (!readLine("", input, sizeof(input))) {
            printError("Please type y or n.");
            continue;
        }
        if (input[1] == '\0' && (input[0] == 'y' || input[0] == 'Y')) {
            return 1;
        }
        if (input[1] == '\0' && (input[0] == 'n' || input[0] == 'N')) {
            return 0;
        }
        printError("Please type y or n.");
    }
}

/* Waits for the user to press Enter, so messages stay visible. */
static void pauseScreen(void)
{
    char input[INPUT_LEN];
    readLine("\nPress Enter to continue...", input, sizeof(input));
}

/* ============================== Validators =============================== */

/* Name: letters, spaces, dots, apostrophes and hyphens; at least 2 letters. */
static int isValidName(const char *text)
{
    int letters = 0;
    const char *p;

    for (p = text; *p != '\0'; p++) {
        if (isalpha((unsigned char)*p)) {
            letters++;
        } else if (*p != ' ' && *p != '.' && *p != '\'' && *p != '-') {
            return 0;
        }
    }
    return letters >= 2;
}

/* Course: letters, digits, spaces and a few symbols (e.g. "B.Tech CSE"). */
static int isValidCourse(const char *text)
{
    const char *p;

    for (p = text; *p != '\0'; p++) {
        if (!isalnum((unsigned char)*p) && *p != ' ' && *p != '.' &&
            *p != '-' && *p != '&' && *p != '(' && *p != ')') {
            return 0;
        }
    }
    return 1;
}

/* Phone: exactly 10 digits. */
static int isValidPhone(const char *text)
{
    int i;

    if (strlen(text) != 10) {
        return 0;
    }
    for (i = 0; i < 10; i++) {
        if (!isdigit((unsigned char)text[i])) {
            return 0;
        }
    }
    return 1;
}

/* Email: simple check -> no spaces, one '@', text before it, a dot after it. */
static int isValidEmail(const char *text)
{
    const char *at = strchr(text, '@');
    const char *dot;
    const char *p;

    if (at == NULL || at == text || strchr(at + 1, '@') != NULL) {
        return 0;
    }
    for (p = text; *p != '\0'; p++) {
        if (isspace((unsigned char)*p)) {
            return 0;
        }
    }
    dot = strrchr(at, '.');
    /* dot must exist, must not be right after '@', and must not be last */
    if (dot == NULL || dot == at + 1 || *(dot + 1) == '\0') {
        return 0;
    }
    return 1;
}

/* ============================ String helpers ============================= */

/* Compares two strings ignoring upper/lower case. Works like strcmp. */
static int compareIgnoreCase(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb) {
            return ca - cb;
        }
        a++;
        b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

/* Returns 1 if 'part' appears anywhere inside 'text' (ignoring case). */
static int containsIgnoreCase(const char *text, const char *part)
{
    size_t partLen = strlen(part);
    size_t i, j;

    if (partLen == 0) {
        return 0;
    }
    for (i = 0; text[i] != '\0'; i++) {
        for (j = 0; j < partLen; j++) {
            if (text[i + j] == '\0' ||
                tolower((unsigned char)text[i + j]) != tolower((unsigned char)part[j])) {
                break;
            }
        }
        if (j == partLen) {
            return 1;
        }
    }
    return 0;
}

/* ============================ Display helpers ============================ */

static void printBanner(void)
{
    printf("\n");
    printf("  ==============================================================\n");
    printf("  ||                                                          ||\n");
    printf("  ||            STUDENT RECORD MANAGEMENT SYSTEM              ||\n");
    printf("  ||                 Built with C Programming                 ||\n");
    printf("  ||                                                          ||\n");
    printf("  ==============================================================\n\n");
}

static void printMenu(void)
{
    printf("\n");
    printBanner();
    printf("  ------------------------ MAIN MENU -------------------------\n");
    printf("    1. Add Student\n");
    printf("    2. Display All Students\n");
    printf("    3. Search Student by ID\n");
    printf("    4. Search Student by Name\n");
    printf("    5. Update Student\n");
    printf("    6. Delete Student\n");
    printf("    7. Sort Students (by ID or Name)\n");
    printf("    8. Show Average Marks\n");
    printf("    9. Show Student with Highest Marks\n");
    printf("    0. Save and Exit\n");
    printf("  ------------------------------------------------------------\n");
}

static void printSectionTitle(const char *title)
{
    printf("\n  ======== %s ========\n\n", title);
}

static void printTableHeader(void)
{
    printf("  %-6s %-22s %-4s %-3s %-16s %-4s %-11s %-28s %-7s\n",
           "ID", "Name", "Age", "Sex", "Course", "Sem", "Phone", "Email", "Marks%");
    printf("  %-6s %-22s %-4s %-3s %-16s %-4s %-11s %-28s %-7s\n",
           "------", "----------------------", "----", "---", "----------------",
           "----", "-----------", "----------------------------", "-------");
}

/* %-22.22s means: left-align in 22 columns and print at most 22 characters,
 * so a very long name cannot break the table. */
static void printTableRow(const Student *s)
{
    printf("  %-6d %-22.22s %-4d %-3c %-16.16s %-4d %-11s %-28.28s %-7.2f\n",
           s->id, s->name, s->age, s->gender, s->course,
           s->semester, s->phone, s->email, s->marks);
}

static void printStudentDetails(const Student *s)
{
    const char *genderText = (s->gender == 'M') ? "Male" :
                             (s->gender == 'F') ? "Female" : "Other";
    char marksText[16];

    snprintf(marksText, sizeof(marksText), "%.2f%%", s->marks);

    printf("  +----------------------------------------------------+\n");
    printf("  |  Student ID : %-36d |\n", s->id);
    printf("  |  Name       : %-36.36s |\n", s->name);
    printf("  |  Age        : %-36d |\n", s->age);
    printf("  |  Gender     : %-36s |\n", genderText);
    printf("  |  Course     : %-36.36s |\n", s->course);
    printf("  |  Semester   : %-36d |\n", s->semester);
    printf("  |  Phone      : %-36s |\n", s->phone);
    printf("  |  Email      : %-36.36s |\n", s->email);
    printf("  |  Marks      : %-36s |\n", marksText);
    printf("  +----------------------------------------------------+\n");
}

static void printSuccess(const char *message)
{
    printf("  [SUCCESS] %s\n", message);
}

static void printError(const char *message)
{
    printf("  [ERROR] %s\n", message);
}

/* =============================== File I/O ================================ */

/*
 * Loads all records from DATA_FILE into the array.
 * Returns 1 if records were loaded (or there is simply nothing to load yet),
 * and 0 if the file was damaged (the damaged file is kept as a .bak copy and
 * the program starts with an empty list instead of crashing).
 */
static int loadStudents(Student students[], int *count)
{
    FILE      *fp;
    FileHeader header;
    long       fileSize;
    int        i, j;

    *count = 0;

    fp = fopen(DATA_FILE, "rb");
    if (fp == NULL) {
        printf("  [INFO] No data file found. Starting with an empty database.\n");
        return 1;
    }

    /* An empty file is fine: it just means "no students yet". */
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    rewind(fp);
    if (fileSize == 0) {
        fclose(fp);
        printf("  [INFO] Data file is empty. Starting with an empty database.\n");
        return 1;
    }

    /* Check the header. */
    if (fread(&header, sizeof(FileHeader), 1, fp) != 1 ||
        memcmp(header.magic, FILE_MAGIC, 4) != 0 ||
        header.count < 0 || header.count > MAX_STUDENTS ||
        fileSize != (long)(sizeof(FileHeader) + (size_t)header.count * sizeof(Student))) {
        goto corrupted;
    }

    /* Read the records. */
    if (header.count > 0 &&
        fread(students, sizeof(Student), (size_t)header.count, fp) != (size_t)header.count) {
        goto corrupted;
    }
    fclose(fp);

    /* Final safety checks on the data itself. */
    for (i = 0; i < header.count; i++) {
        students[i].name[NAME_LEN - 1]     = '\0';
        students[i].course[COURSE_LEN - 1] = '\0';
        students[i].phone[PHONE_LEN - 1]   = '\0';
        students[i].email[EMAIL_LEN - 1]   = '\0';

        if (students[i].id <= 0) {
            goto corrupted_after_close;
        }
        for (j = 0; j < i; j++) {
            if (students[j].id == students[i].id) {
                goto corrupted_after_close;
            }
        }
    }

    *count = header.count;
    printf("  [INFO] Loaded %d student record(s) from file.\n", *count);
    return 1;

corrupted:
    fclose(fp);
corrupted_after_close:
    *count = 0;
    remove(BACKUP_FILE);                    /* ignore error if it is missing  */
    if (rename(DATA_FILE, BACKUP_FILE) == 0) {
        printf("  [WARNING] Data file was damaged. A copy was saved as %s\n", BACKUP_FILE);
    } else {
        printf("  [WARNING] Data file was damaged and could not be backed up.\n");
    }
    printf("  [INFO] Starting with an empty database.\n");
    return 0;
}

/*
 * Saves all records. To protect the old data, we first write everything to a
 * temporary file and only replace the real file when that succeeded.
 * Returns 1 on success, 0 on failure.
 */
static int saveStudents(const Student students[], int count)
{
    FILE      *fp;
    FileHeader header;

    fp = fopen(TEMP_FILE, "wb");
    if (fp == NULL) {
        printError("Could not create the data file. Does the 'data' folder exist,");
        printError("and are you running the program from the project folder?");
        return 0;
    }

    memcpy(header.magic, FILE_MAGIC, 4);
    header.count = count;

    if (fwrite(&header, sizeof(FileHeader), 1, fp) != 1 ||
        (count > 0 && fwrite(students, sizeof(Student), (size_t)count, fp) != (size_t)count)) {
        printError("Failed while writing to the data file.");
        fclose(fp);
        remove(TEMP_FILE);
        return 0;
    }

    if (fclose(fp) != 0) {
        printError("Failed to finish writing the data file.");
        remove(TEMP_FILE);
        return 0;
    }

    /* On Windows, rename() fails if the target exists, so remove it first. */
    remove(DATA_FILE);
    if (rename(TEMP_FILE, DATA_FILE) != 0) {
        printError("Could not replace the old data file.");
        return 0;
    }
    return 1;
}

/* ============================ Core features ============================== */

/* Returns a pointer to the student with this ID, or NULL if not found. */
static Student *findStudentById(Student students[], int count, int id)
{
    int i;

    for (i = 0; i < count; i++) {
        if (students[i].id == id) {
            return &students[i];
        }
    }
    return NULL;
}

/* A. Add a new student. */
static void addStudent(Student students[], int *count)
{
    Student newStudent;
    int     id;

    printSectionTitle("ADD NEW STUDENT");

    if (*count >= MAX_STUDENTS) {
        printError("Database is full. Delete a record before adding a new one.");
        return;
    }

    memset(&newStudent, 0, sizeof(newStudent));

    /* ID must be unique. */
    while (1) {
        promptInt("  Student ID (1-999999)   : ", 1, 999999, &id, 0);
        if (findStudentById(students, *count, id) != NULL) {
            printError("A student with this ID already exists. Use a different ID.");
        } else {
            break;
        }
    }
    newStudent.id = id;

    promptText("  Full name               : ", newStudent.name, NAME_LEN, 0,
               isValidName, "Name may contain only letters, spaces, . ' - (min 2 letters).");
    promptInt("  Age (15-100)            : ", 15, 100, &newStudent.age, 0);
    promptGender(&newStudent.gender, 0);
    promptText("  Course / Branch         : ", newStudent.course, COURSE_LEN, 0,
               isValidCourse, "Course may contain only letters, digits, spaces and . - & ( )");
    promptInt("  Semester (1-8)          : ", 1, 8, &newStudent.semester, 0);
    promptText("  Phone (10 digits)       : ", newStudent.phone, PHONE_LEN, 0,
               isValidPhone, "Phone number must be exactly 10 digits.");
    promptText("  Email address           : ", newStudent.email, EMAIL_LEN, 0,
               isValidEmail, "Enter a valid email, e.g. name@example.com");
    promptFloat("  Marks / Percentage (0-100): ", 0.0f, 100.0f, &newStudent.marks, 0);

    students[*count] = newStudent;
    (*count)++;

    if (saveStudents(students, *count)) {
        printSuccess("Student added and saved successfully.");
    } else {
        /* Saving failed: undo the change so memory and file stay in sync. */
        (*count)--;
        printError("Student was NOT added because the file could not be saved.");
    }
}

/* B. Show every student in a table. */
static void displayAllStudents(const Student students[], int count)
{
    int i;

    printSectionTitle("ALL STUDENT RECORDS");

    if (count == 0) {
        printf("  No student records found. Add a student first (option 1).\n");
        return;
    }

    printTableHeader();
    for (i = 0; i < count; i++) {
        printTableRow(&students[i]);
    }
    printf("\n  Total students: %d\n", count);
}

/* C1. Search by ID. */
static void searchById(Student students[], int count)
{
    int      id;
    Student *found;

    printSectionTitle("SEARCH STUDENT BY ID");

    if (count == 0) {
        printf("  No student records found.\n");
        return;
    }

    promptInt("  Enter student ID to search: ", 1, 999999, &id, 0);
    found = findStudentById(students, count, id);

    if (found == NULL) {
        printError("No student found with that ID.");
    } else {
        printf("\n");
        printStudentDetails(found);
    }
}

/* C2. Search by name (part of the name is enough, case does not matter). */
static void searchByName(const Student students[], int count)
{
    char keyword[INPUT_LEN];
    int  i;
    int  matches = 0;

    printSectionTitle("SEARCH STUDENT BY NAME");

    if (count == 0) {
        printf("  No student records found.\n");
        return;
    }

    while (1) {
        if (!readLine("  Enter name (or part of it): ", keyword, sizeof(keyword))) {
            printError("Input is too long. Please try again.");
        } else if (keyword[0] == '\0') {
            printError("Search text cannot be empty.");
        } else {
            break;
        }
    }

    for (i = 0; i < count; i++) {
        if (containsIgnoreCase(students[i].name, keyword)) {
            printf("\n");
            printStudentDetails(&students[i]);
            matches++;
        }
    }

    if (matches == 0) {
        printError("No student found with that name.");
    } else {
        printf("\n  %d matching student(s) found.\n", matches);
    }
}

/* D. Update a student. Pressing Enter on a field keeps the old value. */
static void updateStudent(Student students[], int count)
{
    int      id;
    Student *target;
    Student  backup;               /* copy used to undo if saving fails       */
    char     gender;

    printSectionTitle("UPDATE STUDENT");

    if (count == 0) {
        printf("  No student records found.\n");
        return;
    }

    promptInt("  Enter ID of the student to update: ", 1, 999999, &id, 0);
    target = findStudentById(students, count, id);

    if (target == NULL) {
        printError("No student found with that ID.");
        return;
    }

    printf("\n  Current details:\n");
    printStudentDetails(target);
    printf("\n  Enter the new value, or just press Enter to keep the current one.\n");
    printf("  (The student ID cannot be changed.)\n\n");

    backup = *target;              /* structs can be copied with '='          */

    promptText("  Full name               : ", target->name, NAME_LEN, 1,
               isValidName, "Name may contain only letters, spaces, . ' - (min 2 letters).");
    promptInt("  Age (15-100)            : ", 15, 100, &target->age, 1);
    if (promptGender(&gender, 1)) {
        target->gender = gender;
    }
    promptText("  Course / Branch         : ", target->course, COURSE_LEN, 1,
               isValidCourse, "Course may contain only letters, digits, spaces and . - & ( )");
    promptInt("  Semester (1-8)          : ", 1, 8, &target->semester, 1);
    promptText("  Phone (10 digits)       : ", target->phone, PHONE_LEN, 1,
               isValidPhone, "Phone number must be exactly 10 digits.");
    promptText("  Email address           : ", target->email, EMAIL_LEN, 1,
               isValidEmail, "Enter a valid email, e.g. name@example.com");
    promptFloat("  Marks / Percentage (0-100): ", 0.0f, 100.0f, &target->marks, 1);

    if (saveStudents(students, count)) {
        printSuccess("Student record updated successfully.");
    } else {
        *target = backup;          /* put the old values back                 */
        printError("Update cancelled because the file could not be saved.");
    }
}

/* E. Delete a student after asking for confirmation. */
static void deleteStudent(Student students[], int *count)
{
    int      id;
    Student *target;
    int      index, i;
    Student  removed;

    printSectionTitle("DELETE STUDENT");

    if (*count == 0) {
        printf("  No student records found.\n");
        return;
    }

    promptInt("  Enter ID of the student to delete: ", 1, 999999, &id, 0);
    target = findStudentById(students, *count, id);

    if (target == NULL) {
        printError("No student found with that ID.");
        return;
    }

    printf("\n");
    printStudentDetails(target);
    printf("\n");

    if (!confirm("  Are you sure you want to delete this record?")) {
        printf("  Deletion cancelled. No changes were made.\n");
        return;
    }

    /* Pointer subtraction gives the position of the record in the array. */
    index   = (int)(target - students);
    removed = *target;

    /* Shift every later record one place to the left to fill the gap. */
    for (i = index; i < *count - 1; i++) {
        students[i] = students[i + 1];
    }
    (*count)--;

    if (saveStudents(students, *count)) {
        printSuccess("Student record deleted successfully.");
    } else {
        /* Undo: shift right again and restore the removed record. */
        for (i = *count; i > index; i--) {
            students[i] = students[i - 1];
        }
        students[index] = removed;
        (*count)++;
        printError("Deletion cancelled because the file could not be saved.");
    }
}

/* F1. Sort by ID or by name (selection sort). */
static void sortStudents(Student students[], int count)
{
    int     choice;
    int     i, j, best;
    Student temp;

    printSectionTitle("SORT STUDENTS");

    if (count < 2) {
        printf("  Need at least 2 records to sort.\n");
        return;
    }

    printf("    1. Sort by ID (ascending)\n");
    printf("    2. Sort by Name (A-Z)\n\n");
    promptInt("  Enter your choice (1-2): ", 1, 2, &choice, 0);

    /* Selection sort: repeatedly pick the smallest remaining item and swap
     * it into position i. */
    for (i = 0; i < count - 1; i++) {
        best = i;
        for (j = i + 1; j < count; j++) {
            int isSmaller;
            if (choice == 1) {
                isSmaller = students[j].id < students[best].id;
            } else {
                isSmaller = compareIgnoreCase(students[j].name, students[best].name) < 0;
            }
            if (isSmaller) {
                best = j;
            }
        }
        if (best != i) {
            temp          = students[i];
            students[i]   = students[best];
            students[best] = temp;
        }
    }

    if (saveStudents(students, count)) {
        printSuccess(choice == 1 ? "Students sorted by ID." : "Students sorted by name.");
    }
    printf("\n");
    displayAllStudents(students, count);
}

/* F2. Average marks of all students. */
static void showAverageMarks(const Student students[], int count)
{
    double total = 0.0;
    int    i;

    printSectionTitle("AVERAGE MARKS");

    if (count == 0) {
        printf("  No student records found.\n");
        return;
    }

    for (i = 0; i < count; i++) {
        total += students[i].marks;
    }
    printf("  Number of students : %d\n", count);
    printf("  Average marks      : %.2f%%\n", total / count);
}

/* F3. Student(s) with the highest marks (shows all students in a tie). */
static void showTopStudent(const Student students[], int count)
{
    float highest;
    int   i;

    printSectionTitle("HIGHEST MARKS");

    if (count == 0) {
        printf("  No student records found.\n");
        return;
    }

    highest = students[0].marks;
    for (i = 1; i < count; i++) {
        if (students[i].marks > highest) {
            highest = students[i].marks;
        }
    }

    printf("  Highest marks: %.2f%%\n\n", highest);
    for (i = 0; i < count; i++) {
        if (students[i].marks == highest) {
            printStudentDetails(&students[i]);
            printf("\n");
        }
    }
}
