# Test Cases

Start every test from the project root. Delete `data/students.dat` first when a clean start is needed.

| # | Test | Steps | Expected result |
|---|------|-------|-----------------|
| 1 | Add a student | Menu 1; ID `101`, name `Asha Sharma`, age `19`, gender `f`, course `B.E. CSE`, sem `2`, phone `9876543210`, email `asha@example.com`, marks `88.5` | `[SUCCESS] Student added and saved successfully.` Gender is stored as `F`. |
| 2 | Display all (with data) | Menu 2 | Aligned table with the student and `Total students: 1`. |
| 3 | Display all (empty) | Delete `data/students.dat`, restart, menu 2 | `No student records found. Add a student first (option 1).` |
| 4 | Search by ID, existing | Menu 3, `101` | Full detail box for Asha Sharma. |
| 5 | Search by ID, missing | Menu 3, `999` | `[ERROR] No student found with that ID.` |
| 6 | Search by name, existing | Menu 4, `sha` | Asha Sharma is shown (partial, case-insensitive). |
| 7 | Search by name, missing | Menu 4, `zzz` | `[ERROR] No student found with that name.` |
| 8 | Update | Menu 5, ID `101`, press Enter on all fields except marks, type `92` | Only marks change. Other fields keep their old values. |
| 9 | Update invalid ID | Menu 5, `555` | `[ERROR] No student found with that ID.` |
| 10 | Delete, cancel | Menu 6, `101`, answer `n` | `Deletion cancelled. No changes were made.` Record still present. |
| 11 | Delete, confirm | Add a second student (ID `102`). Menu 6, `101`, answer `y` | Success message. Menu 2 shows only student 102. |
| 12 | Duplicate ID | Menu 1 with an existing ID | `[ERROR] A student with this ID already exists.` and the ID is asked again. |
| 13 | Invalid numbers | Age `abc`, `5`, `150`; semester `9`; marks `-1`, `101` | Each gives an error and the question repeats. |
| 14 | Invalid text | Name `R2D2`; phone `12345`; email `bad@` | Each gives an error and the question repeats. |
| 15 | Empty input on required field | Press Enter on name | `[ERROR] This field cannot be empty.` |
| 16 | Very long input | Paste 200 characters as a name | `[ERROR] Input is too long.` No crash, no leftover text in next question. |
| 17 | Invalid menu choice | Enter `abc`, `12`, `-1` | Error and the menu is asked again. |
| 18 | Persistence | Add students, choose 0 to exit, run again, menu 2 | Same students appear, and startup shows `Loaded N student record(s)`. |
| 19 | Missing data file | Delete `data/students.dat`, run | `[INFO] No data file found. Starting with an empty database.` |
| 20 | Empty data file | Create an empty `data/students.dat`, run | `[INFO] Data file is empty...` and no crash. |
| 21 | Corrupted data file | Put random text in `data/students.dat`, run | `[WARNING] Data file was damaged...`, the old file is kept as `data/students.corrupt.bak`, program starts empty. |
| 22 | Sort | Add IDs 30, 10, 20 with names Zoya, Amit, Neha. Menu 7 option 1, then option 2 | IDs ordered 10, 20, 30; then names Amit, Neha, Zoya. |
| 23 | Average / highest | Marks 60, 80, 100 then menu 8 and 9 | Average `80.00%`; highest `100.00%` student shown. |
| 24 | Missing `data` folder | Rename the `data` folder, run, add a student | Clear error telling you to check the `data` folder and the run location. |
