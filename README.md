# Intelligent Course Scheduler (C++ + MySQL) — Windows

Generates a conflict-free course plan using **topological sort** (for prerequisites) and **interval conflict checks** (per day). Data is stored in **MySQL**.

## Setup (Windows)

1) **Install MySQL Server 8.0** and note your root password.  
2) **Install MySQL Connector/C** (or ensure `libmysql.lib` and `mysql.h` are available).  
3) **Import schema & sample data**  
   - Open **MySQL Shell** or **Workbench** and run:
     ```sql
     SOURCE path\to\schema.sql;
     ```
4) **Build with MSVC (Developer Command Prompt for VS):**
   Adjust include/lib paths if different.
   ```bat
   cl /EHsc main.cpp /I"C:\Program Files\MySQL\MySQL Server 8.0\include" ^
      /link /LIBPATH:"C:\Program Files\MySQL\MySQL Server 8.0\lib" libmysql.lib
   ```
   Or with Connector/C:
   ```bat
   cl /EHsc main.cpp /I"C:\Program Files\MySQL\MySQL Connector C 8.0\include" ^
      /link /LIBPATH:"C:\Program Files\MySQL\MySQL Connector C 8.0\lib" libmysql.lib
   ```
5) **Run**
   ```bat
   main.exe
   ```
   Update credentials inside `main.cpp` (host/user/password).

## Files
- `schema.sql` — tables & sample data
- `main.cpp` — C++17 CLI app (MySQL C API)
- `README.md` — this file

